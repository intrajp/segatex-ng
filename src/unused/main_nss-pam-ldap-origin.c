/*
   nslcd.c - ldap local connection daemon

   Copyright (C) 2006 West Consulting
   Copyright (C) 2006-2017 Arthur de Jong
   Copyright (C) 2017 Shintaro Fujiwara 

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA
*/

//#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <grp.h>
#include <pthread.h>
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>

//#include "nslcd.h"
//#include "log.h"
//#include "cfg.h"
//#include "common.h"
//#include "compat/attrs.h"
//#include "compat/getpeercred.h"
//#include "compat/socket.h"
////#include "getopt_long.h"
#include "daemonize.c"
#include "main.h"
#include "cfg.c"

/* read timeout is half a second because clients should send their request
   quickly, write timeout is 60 seconds because clients could be taking some
   time to process the results */
#define READ_TIMEOUT 500
#define WRITE_TIMEOUT 60 * 1000

/* buffer sizes for I/O */
#define READBUFFER_MINSIZE 32
#define READBUFFER_MAXSIZE 64
#define WRITEBUFFER_MINSIZE 1024
#define WRITEBUFFER_MAXSIZE 1 * 1024 * 1024

/* adjust the oom killer score */
#define OOM_SCORE_ADJ_FILE "/proc/self/oom_score_adj"
#define OOM_SCORE_ADJ "-1000"

/* flag to indicate if we are in debugging mode */
static int segatexd_debugging = 0;

/* flag to indicate we shouldn't daemonize */
static int segatexd_nofork = 0;

/* flag to indicate user requested the --check option */
static int segatexd_checkonly = 0;

/* the flag to indicate that a signal was received */
static volatile int segatexd_receivedsignal = 0;

/* the server socket used for communication */
static int segatexd_serversocket = -1;

/* thread ids of all running threads */
static pthread_t *segatexd_threads;

/* if we don't have clearenv() we have to do this the hard way */
#ifndef HAVE_CLEARENV

/* the definition of the environment */
extern char **environ;

/* the environment we want to use */
static char *sane_environment[] = {
  "HOME=/",
  "TMPDIR=/tmp",
  "LDAPNOINIT=1",
  NULL
};

#endif /* not HAVE_CLEARENV */

/* display version information */
static void display_version(FILE *fp)
{
  fprintf(fp, "%s\n", PACKAGE_STRING);
  fprintf(fp, "Written by Luke Howard and Arthur de Jong.\n\n");
  fprintf(fp, "Copyright (C) 1997-2017 Luke Howard, Arthur de Jong and West Consulting\n"
              "This is free software; see the source for copying conditions.  There is NO\n"
              "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
}

/* display usage information */
static void display_usage(FILE *fp, const char *program_name)
{
  fprintf(fp, "Usage: %s [OPTION]...\n", program_name);
  fprintf(fp, "Name Service LDAP connection daemon.\n");
  fprintf(fp, "  -c, --check        check if the daemon already is running\n");
  fprintf(fp, "  -d, --debug        don't fork and print debugging to stderr\n");
  fprintf(fp, "  -n, --nofork       don't fork\n");
  fprintf(fp, "      --help         display this help and exit\n");
  fprintf(fp, "      --version      output version information and exit\n");
  fprintf(fp, "\n" "Report bugs to <%s>.\n", PACKAGE_BUGREPORT);
}

/* the definition of options for getopt(). see getopt(2) */
static struct option const segatexd_options[] = {
  {"check",   no_argument, NULL, 'c'},
  {"debug",   no_argument, NULL, 'd'},
  {"nofork",  no_argument, NULL, 'n'},
  {"help",    no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'},
  {NULL,      0,           NULL, 0}
};
#define NSLCD_OPTIONSTRING "cndhV"

/* parse command line options and save settings in struct  */
static void parse_cmdline(int argc, char *argv[])
{
  int optc;
  while ((optc = getopt_long(argc, argv, NSLCD_OPTIONSTRING, segatexd_options, NULL)) != -1)
  {
    switch (optc)
    {
      case 'c': /* -c, --check        check if the daemon already is running */
        segatexd_checkonly = 1;
        break;
      case 'd': /* -d, --debug        don't fork and print debugging to stderr */
        segatexd_debugging++;
        //log_setdefaultloglevel(LOG_DEBUG);
        break;
      case 'n': /* -n, --nofork       don't fork */
        segatexd_nofork++;
        break;
      case 'h': /*     --help         display this help and exit */
        display_usage(stdout, argv[0]);
        exit(EXIT_SUCCESS);
      case 'V': /*     --version      output version information and exit */
        display_version(stdout);
        exit(EXIT_SUCCESS);
      case ':': /* missing required parameter */
      case '?': /* unknown option character or extraneous parameter */
      default:
        fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  /* check for remaining arguments */
  if (optind < argc)
  {
    fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[optind]);
    fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
    exit(EXIT_FAILURE);
  }
}

/* signal handler for storing information on received signals */
static void sig_handler(int signum)
{
  /* just save the signal to indicate that we're stopping */
  segatexd_receivedsignal = signum;
}

/* do some cleaning up before terminating */
static void exithandler(void)
{
  /* close socket if it's still in use */
  if (segatexd_serversocket >= 0)
  {
    if (close(segatexd_serversocket))
      //log_log(LOG_WARNING, "problem closing server socket (ignored): %s",
      printf("problem closing server socket (ignored): %s",
              strerror(errno));
  }
  /* remove existing named socket */
  if (unlink(SEGATEXD_SOCKET) < 0)
  {
    //log_log(LOG_DEBUG, "unlink() of " SEGATEXD_SOCKET " failed (ignored): %s",
    printf("unlink() of " SEGATEXD_SOCKET " failed (ignored): %s",
            strerror(errno));
  }
  /* remove pidfile */
  if (unlink(SEGATEXD_PIDFILE) < 0)
  {
    //log_log(LOG_DEBUG, "unlink() of " SEGATEXD_PIDFILE " failed (ignored): %s",
    printf("unlink() of " SEGATEXD_PIDFILE " failed (ignored): %s",
            strerror(errno));
  }
  /* log exit */
  //log_log(LOG_INFO, "version %s bailing out", VERSION);
  printf("version %s bailing out", VERSION);
}

/* create the directory for the specified file to reside in */
static void mkdirname(const char *filename)
{
  char *tmpname, *path;
  tmpname = strdup(filename);
  if (tmpname == NULL)
    return;
  path = dirname(tmpname);
  if (mkdir(path, (mode_t)0755) == 0)
  {
      printf("Created directory");
  }
  /* if directory was just created, set correct ownership */
  /*
  if (mkdir(path, (mode_t)0755) == 0)
  {
    if (lchown(path, segatexd_cfg->uid, nslcd_cfg->gid) < 0)
      printf("problem setting permissions for %s: %s",
              path, strerror(errno));
  }
  */
  free(tmpname);
}

/* returns a socket ready to answer requests from the client,
   exit()s on error */
static int create_socket(const char *filename)
{
  int sock;
  int i;
  struct sockaddr_un addr;
  /* create a socket */
  if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    //log_log(LOG_ERR, "cannot create socket: %s", strerror(errno));
    printf("cannot create socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (sock >= (int)FD_SETSIZE)
  {
    //log_log(LOG_ERR, "socket file descriptor number too high (%d)", sock);
    printf("socket file descriptor number too high (%d)", sock);
    exit(EXIT_FAILURE);
  }
  /* remove existing named socket */
  if (unlink(filename) < 0)
  {
    //log_log(LOG_DEBUG, "unlink() of %s failed (ignored): %s",
            //filename, strerror(errno));
    printf("unlink() of %s failed (ignored): %s",
            filename, strerror(errno));
  }
  /* do not block on accept() */
  if ((i = fcntl(sock, F_GETFL, 0)) < 0)
  {
    //log_log(LOG_ERR, "fctnl(F_GETFL) failed: %s", strerror(errno));
    printf("fctnl(F_GETFL) failed: %s", strerror(errno));
    if (close(sock))
      //log_log(LOG_WARNING, "problem closing socket: %s", strerror(errno));
      printf("problem closing socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (fcntl(sock, F_SETFL, i | O_NONBLOCK) < 0)
  {
    //log_log(LOG_ERR, "fctnl(F_SETFL,O_NONBLOCK) failed: %s", strerror(errno));
    printf("fctnl(F_SETFL,O_NONBLOCK) failed: %s", strerror(errno));
    if (close(sock))
      //log_log(LOG_WARNING, "problem closing socket: %s", strerror(errno));
      printf("problem closing socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* create the directory if needed */
  mkdirname(filename);
  /* create socket address structure */
  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, filename, sizeof(addr.sun_path));
  addr.sun_path[sizeof(addr.sun_path) - 1] = '\0';
  /* bind to the named socket */
  if (bind(sock, (struct sockaddr *)&addr, SUN_LEN(&addr)))
  {
    //log_log(LOG_ERR, "bind() to %s failed: %s", filename, strerror(errno));
    printf("bind() to %s failed: %s", filename, strerror(errno));
    if (close(sock))
      //log_log(LOG_WARNING, "problem closing socket: %s", strerror(errno));
      printf("problem closing socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* close the file descriptor on exec */
  if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0)
  {
    //log_log(LOG_ERR, "fctnl(F_SETFL,FD_CLOEXEC) on %s failed: %s",
            //filename, strerror(errno));
    printf("fctnl(F_SETFL,FD_CLOEXEC) on %s failed: %s",
            filename, strerror(errno));
    if (close(sock))
      //log_log(LOG_WARNING, "problem closing socket: %s", strerror(errno));
      printf("problem closing socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* set permissions of socket so anybody can do requests */
  /* Note: we use chmod() here instead of fchmod() because
     fchmod does not work on sockets
     http://www.opengroup.org/onlinepubs/009695399/functions/fchmod.html
     http://lkml.org/lkml/2005/5/16/11 */
  if (chmod(filename, (mode_t)0666))
  {
    //log_log(LOG_ERR, "chmod(0666) of %s failed: %s",
     //       filename, strerror(errno));
    printf("chmod(0666) of %s failed: %s",
            filename, strerror(errno));
    if (close(sock))
      //log_log(LOG_WARNING, "problem closing socket: %s", strerror(errno));
      printf("problem closing socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* start listening for connections */
  if (listen(sock, SOMAXCONN) < 0)
  {
    //log_log(LOG_ERR, "listen() failed: %s", strerror(errno));
    printf("listen() failed: %s", strerror(errno));
    if (close(sock))
      //log_log(LOG_WARNING, "problem closing socket: %s", strerror(errno));
      printf("problem closing socket: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* we're done */
  return sock;
}

/* test to see if we can lock the specified file */
static int is_locked(const char *filename)
{
  int fd;
  if (filename != NULL)
  {
    errno = 0;
    if ((fd = open(filename, O_RDWR, 0644)) < 0)
    {
      if (errno == ENOENT)
        return 0; /* if file doesn't exist it cannot be locked */
      //log_log(LOG_ERR, "cannot open lock file (%s): %s", filename, strerror(errno));
      printf("cannot open lock file (%s): %s", filename, strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (lockf(fd, F_TEST, 0) < 0)
    {
      if (close(fd))
        //log_log(LOG_WARNING, "problem closing fd: %s", strerror(errno));
        printf("problem closing fd: %s", strerror(errno));
      return -1;
    }
    if (close(fd))
      //log_log(LOG_WARNING, "problem closing fd: %s", strerror(errno));
      printf("problem closing fd: %s\n", strerror(errno));
  }
  return 0;
}

/* write the current process id to the specified file */
static void create_pidfile(const char *filename)
{
  int fd;
  char buffer[20];
  if (filename != NULL)
  {
    mkdirname(filename);
    if ((fd = open(filename, O_RDWR | O_CREAT, 0644)) < 0)
    {
      //log_log(LOG_ERR, "cannot create pid file (%s): %s",
       //       filename, strerror(errno));
      printf("cannot create pid file (%s): %s\n",
              filename, strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (lockf(fd, F_TLOCK, 0) < 0)
    {
      //log_log(LOG_ERR, "cannot lock pid file (%s): %s",
       //       filename, strerror(errno));
      printf("cannot lock pid file (%s): %s\n",
              filename, strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (ftruncate(fd, 0) < 0)
    {
      //log_log(LOG_ERR, "cannot truncate pid file (%s): %s",
       //       filename, strerror(errno));
      printf("cannot truncate pid file (%s): %s\n",
              filename, strerror(errno));
      exit(EXIT_FAILURE);
    }
    mysnprintf(buffer, sizeof(buffer), "%lu\n", (unsigned long int)getpid());
    if (write(fd, buffer, strlen(buffer)) != (int)strlen(buffer))
    {
      //log_log(LOG_ERR, "error writing pid file (%s): %s",
       //       filename, strerror(errno));
      printf("error writing pid file (%s): %s\n",
              filename, strerror(errno));
      exit(EXIT_FAILURE);
    }
    /* we keep the pidfile open so the lock remains valid */
  }
}

/* try to install signal handler and check result */
static void install_sighandler(int signum, void (*handler) (int))
{
  struct sigaction act;
  memset(&act, 0, sizeof(struct sigaction));
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  if (sigaction(signum, &act, NULL) != 0)
  {
    //log_log(LOG_ERR, "error installing signal handler for '%s': %s",
     //       signame(signum), strerror(errno));
    printf("error installing signal handler: %s");
    exit(EXIT_FAILURE);
  }
}

/* simple wrapper around snptintf() to return non-zero in case
   of any failure (but always keep string 0-terminated) */
int mysnprintf(char *buffer, size_t buflen, const char *format, ...)
{
  int res;
  va_list ap;
  /* do snprintf */
  //va_start(ap, format);
  res = vsnprintf(buffer, buflen, format, ap);
  //va_end(ap);
  /* NULL-terminate the string just to be on the safe side */
  buffer[buflen - 1] = '\0';
  /* check if the string was completely written */
  return ((res < 0) || (((size_t)res) >= buflen));
}



/* the main program... */
int main(int argc, char *argv[])
{
  int i;
  sigset_t signalmask, oldmask;
  /* block all these signals so our worker threads won't handle them */
  sigemptyset(&signalmask);
  sigaddset(&signalmask, SIGHUP);
  sigaddset(&signalmask, SIGINT);
  sigaddset(&signalmask, SIGQUIT);
  sigaddset(&signalmask, SIGABRT);
  sigaddset(&signalmask, SIGPIPE);
  sigaddset(&signalmask, SIGTERM);
  sigaddset(&signalmask, SIGUSR1);
  sigaddset(&signalmask, SIGUSR2);
  //pthread_sigmask(SIG_BLOCK, &signalmask, &oldmask);
  /* close all file descriptors (except stdin/out/err) */
  //daemonize_closefds();
  /* parse the command line */
  parse_cmdline(argc, argv);
  /* clean the environment */
  /* disable the nss_ldap module for this process */
  //disable_nss_ldap();
  //////////////////////////////////////////
  /* read configuration file */
  cfg_init(SEGATEXD_CONF_PATH);
  //////////////////////////////////////////
  /* set default mode for pidfile and socket */
  (void)umask((mode_t)0022);
  /* see if someone already locked the pidfile
     if --check option was given exit TRUE if daemon runs
     (pidfile locked), FALSE otherwise */
  if (segatexd_checkonly)
  {
    if (is_locked(SEGATEXD_PIDFILE))
    {
      //log_log(LOG_DEBUG, "pidfile (%s) is locked", SEGATEXD_PIDFILE);
      printf("pidfile (%s) is locked", SEGATEXD_PIDFILE);
      exit(EXIT_SUCCESS);
    }
    else
    {
      //log_log(LOG_DEBUG, "pidfile (%s) is not locked", SEGATEXD_PIDFILE);
      printf("pidfile (%s) is not locked", SEGATEXD_PIDFILE);
      exit(EXIT_FAILURE);
    }
  }
  /* change directory */
  if (chdir("/") != 0)
  {
    //log_log(LOG_ERR, "chdir failed: %s", strerror(errno));
    printf("chdir failed: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* normal check for pidfile locked */
  if (is_locked(SEGATEXD_PIDFILE))
  {
    //log_log(LOG_ERR, "nslcd may already be active, cannot acquire lock (%s): %s",
     //       NSLCD_PIDFILE, strerror(errno));
    printf("nslcd may already be active, cannot acquire lock (%s): %s",
            SEGATEXD_PIDFILE, strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* daemonize */
  if ((!segatexd_debugging) && (!segatexd_nofork))
  {
    errno = 0;
    if (daemonize_daemon() != 0)
    {
      //log_log(LOG_ERR, "unable to daemonize: %s", strerror(errno));
      printf("unable to daemonize: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  /* write pidfile */
  create_pidfile(SEGATEXD_PIDFILE);
  /* log start */
  //log_log(LOG_INFO, "version %s starting", VERSION);
  printf("version %s starting", VERSION);
  /* install handler to close stuff off on exit and log notice */
  if (atexit(exithandler))
  {
    //log_log(LOG_ERR, "atexit() failed: %s", strerror(errno));
    printf("atexit() failed: %s", strerror(errno));
    daemonize_ready(EXIT_FAILURE, "atexit() failed\n");
    exit(EXIT_FAILURE);
  }
  /*start worker threads*/
  segatexd_threads = (pthread_t *)malloc(segatexd_cfg->threads * sizeof(pthread_t));
  if (segatexd_threads == NULL)
  {
    //log_log(LOG_CRIT, "main(): malloc() failed to allocate memory");
    printf("main(): malloc() failed to allocate memory");
    daemonize_ready(EXIT_FAILURE, "malloc() failed to allocate memory\n");
    exit(EXIT_FAILURE);
  }
  static void *worker=NULL;
  for (i = 0; i < segatexd_cfg->threads; i++)
  {
    if (pthread_create(&segatexd_threads[i], NULL, worker, NULL))
    {
      //log_log(LOG_ERR, "unable to start worker thread %d: %s",
       //       i, strerror(errno));
      printf("unable to start worker thread %d: %s",
              i, strerror(errno));
      daemonize_ready(EXIT_FAILURE, "unable to start worker thread\n");
      exit(EXIT_FAILURE);
    }
  }
  /* install signal handlers for some signals */
  install_sighandler(SIGHUP, sig_handler);
  install_sighandler(SIGINT, sig_handler);
  install_sighandler(SIGQUIT, sig_handler);
  install_sighandler(SIGABRT, sig_handler);
  install_sighandler(SIGPIPE, SIG_IGN);
  install_sighandler(SIGTERM, sig_handler);
  install_sighandler(SIGUSR1, sig_handler);
  install_sighandler(SIGUSR2, SIG_IGN);
  /* signal the starting process to exit because we can provide services now */
  daemonize_ready(EXIT_SUCCESS, NULL);
  /* enable receiving of signals */
  //pthread_sigmask(SIG_SETMASK, &oldmask, NULL);
  /* wait until we received a signal */
  while ((segatexd_receivedsignal == 0) || (segatexd_receivedsignal == SIGUSR1))
  {
    sleep(INT_MAX); /* sleep as long as we can or until we receive a signal */
    if (segatexd_receivedsignal == SIGUSR1)
    {
      //log_log(LOG_INFO, "caught signal %s (%d), refresh retries",
       //       signame(nslcd_receivedsignal), nslcd_receivedsignal);
      printf("caught signal");
      //myldap_immediate_reconnect();
      segatexd_receivedsignal = 0;
    }
  }
  /* print something about received signal */
  //log_log(LOG_INFO, "caught signal %s (%d), shutting down",
   //       signame(nslcd_receivedsignal), nslcd_receivedsignal);
  printf("caught signal shutting down");
  /* cancel all running threads */
  for (i = 0; i < segatexd_cfg->threads; i++)
    //if (pthread_cancel(segatexd_threads[i]))
      //log_log(LOG_WARNING, "failed to stop thread %d (ignored): %s",
              //i, strerror(errno));
     // printf("failed to stop thread %d (ignored): %s",
     printf("thread");
      //        i, strerror(errno));
  /* close server socket to trigger failures in threads waiting on accept() */
  close(segatexd_serversocket);
  segatexd_serversocket = -1;
  /* if we can, wait a few seconds for the threads to finish */
  for (i = 0; i < segatexd_cfg->threads; i++)
  {
    //if (pthread_kill(segatexd_threads[i], 0) == 0)
      //log_log(LOG_ERR, "thread %d is still running, shutting down anyway", i);
      //printf("thread %d is still running, shutting down anyway", i);
      printf("thread %d is running", i);
  }
  /* we're done */
  return EXIT_SUCCESS;
}

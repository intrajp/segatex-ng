/*
   daemoninze.c - functions for properly daemonising an application

   This file contains the contents of segatex-ng.

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
/*
   -- from nslcd daemonize.h--
   To properly run as a daemon an application should:

   - close all open file descriptors (see daemonize_closefds() for that)
   - (re)set proper signal handlers and signal mask
   - sanitise the environment
   - fork() / setsid() / fork() to detach from terminal, become process
     leader and run in the background (see daemonize_demon() for that)
   - reconnect stdin/stdout/stderr to /dev/null (see
     daemonize_redirect_stdio() for that)
   - set the umask to a reasonable value
   - chdir(/) to avoid locking any mounts
   - drop privileges as appropriate
   - chroot() if appropriate
   - create and lock a pidfile
   - exit the starting process if initialisation is complete (see
     daemonize_ready() for that)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "../segatexd.h"
#include "cfg.c"

static int running = 0;
static int delay = 1;
static int counter = 0;
static char *pid_file_name = "/var/run/segatexd/segatexd.pid";
static int pid_fd = -1;
static char *app_name = NULL;
static FILE *log_stream;


/**
 * \brief Callback function for handling signals.
 * \param	sig	identifier of signal
 */
void handle_signal(int sig)
{
    printf("handle_signal was called !\n");

    if (sig == SIGINT) {
        fprintf(log_stream, "Debug: stopping daemon ...\n");
        printf("Debug: stopping daemon ...\n");
        /* Unlock and close lockfile */
        if (pid_fd != -1) {
            lockf(pid_fd, F_ULOCK, 0);
            close(pid_fd);
        }
        /* Try to delete lockfile */
        if (pid_file_name != NULL) {
            unlink(pid_file_name);
        }
        running = 0;
        /* Reset signal handling to default behavior */
        signal(SIGINT, SIG_DFL);
    } else if (sig == SIGHUP) {
        fprintf(log_stream, "Debug: reloading daemon config file ...\n");
        printf("Debug: reloading daemon config file ...\n");
        /* clear configuration */
        //cfg_init(fname);
        /* read configfile */
        cfg_read(fname,segatexd_cfg);
    } else if (sig == SIGCHLD) {
        fprintf(log_stream, "Debug: received SIGCHLD signal\n");
        printf("Debug: received SIGCHLD signal\n");
    }
}

/**
 * \brief This function will daemonize this app
 */
static void daemonize()
{
    printf("daemonize process started !\n");

    pid_t pid = 0;
    pid_t pid2 = 0;
    int fd;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0) {
        printf("daemonize process fork failed !\n");
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        printf("daemonize process fork succeeded !\n");
        exit(EXIT_SUCCESS);
    }

    /* On success: The child process becomes session leader */
    if (setsid() < 0) {
        printf("daemonize process setsid failed !\n");
        exit(EXIT_FAILURE);
    }

    /* Ignore signal sent from child to parent process */
    signal(SIGCHLD, SIG_IGN);

    /* Fork off for the second time*/
    pid2 = fork();

    /* An error occurred */
    if (pid2 < 0) {
        printf("daemonize process second fork failed !\n");
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid2 > 0) {
        printf("daemonize process second fork succeeded !\n");
        exit(EXIT_SUCCESS);
    }

    /* Set new file permissions */
    umask(0);
    printf("daemonize process set new file permissions !\n");

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");
    printf("daemonize process chdir to / !\n");

    /* Close all open file descriptors */
    //for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
    //    //printf("daemonize process close file descriptor:%d !\n",fd);
    //    close(fd);
    //}
    //printf("daemonize process closed file descriptors !\n");

    /* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
    stdin = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w+");
    stderr = fopen("/dev/null", "w+");

    /* Try to write PID of daemon to lockfile */
    if (pid_file_name != NULL)
    {
        char str[256];
        pid_fd = open(pid_file_name, O_RDWR|O_CREAT, 0640);
        if (pid_fd < 0) {
            /* Can't open lockfile */
            exit(EXIT_FAILURE);
        }
        if (lockf(pid_fd, F_TLOCK, 0) < 0) {
            /* Can't lock file */
            exit(EXIT_FAILURE);
        }
        /* Get current PID */
        sprintf(str, "%d\n", getpid());
        /* Write PID to lockfile */
        write(pid_fd, str, strlen(str));
        printf("daemonize process write lockfile succeeded !\n");
    }
    printf("daemonize process succeeded !\n");
}

/**
 * \brief Print help for this application
 */
void print_help(void)
{
    printf("\n Usage: %s [OPTIONS]\n\n", app_name);
    printf("  Options:\n");
    printf("   -h --help                 Print this help\n");
    printf("   -c --conf_file filename   Read configuration from the file\n");
    printf("   -t --test_conf filename   Test configuration file\n");
    printf("   -l --log_file  filename   Write logs to the file\n");
    printf("   -d --daemon               Daemonize this application\n");
    printf("   -p --pid_file  filename   PID file used by daemonized app\n");
    printf("\n");
}

/*
 *  daemoninze.c - functions for properly daemonising an application
 *
 *  This file contains the contents of segatex-ng.
 *
 *  Copyright (C) 2017-2018 Shintaro Fujiwara
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301 USA
*/
/*
 *  -- from nslcd daemonize.h--
 *  To properly run as a daemon an application should:
 *
 *  - close all open file descriptors (see daemonize_closefds() for that)
 *  - (re)set proper signal handlers and signal mask
 *  - sanitise the environment
 *  - fork() / setsid() / fork() to detach from terminal, become process
 *    leader and run in the background (see daemonize_demon() for that)
 *  - reconnect stdin/stdout/stderr to /dev/null (see
 *    daemonize_redirect_stdio() for that)
 *  - set the umask to a reasonable value
 *  - chdir(/) to avoid locking any mounts
 *  - drop privileges as appropriate
 *  - chroot() if appropriate
 *  - create and lock a pidfile
 *  - exit the starting process if initialisation is complete (see
 *    daemonize_ready() for that)
*/

#define GLOBAL_VALUE_DEFINE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../segatexd.h"
#include "cfg.c"

static int running = 0;
static int delay = 120;
static int counter = 0;
//static char *pid_file_name = "/var/run/segatexd/segatexd.pid";
static int pid_fd = -1;
static char *app_name = "segatexd";
static FILE *log_stream;
const char msg_sigint [ 30 ] ="segatexd caught SIGINT\n";
const char msg_sighup [ 30 ] ="segatexd caught SIGHUP\n";
const char msg_sigchld [ 30 ] ="segatexd caught SIGCHLD\n";
int SIG_VALUE;

/* brief This function will daemonize this app */

static void daemonize ( )
{
    pid_t pid = 0;
    pid_t pid2 = 0;
    int fd;

    /* Fork off the parent process */
    pid = fork ( );
    /* An error occurred */
    if ( pid < 0 ) {
        exit ( EXIT_FAILURE );
    }
    /* Success: Let the parent terminate */
    if ( pid > 0 ) {
        exit ( EXIT_SUCCESS );
    }
    /* On success: The child process becomes session leader */
    if ( setsid ( ) < 0 ) {
        exit ( EXIT_FAILURE );
    }
    /* Ignore signal sent from child to parent process */
    signal ( SIGCHLD, SIG_IGN );
    /* Fork off for the second time*/
    pid2 = fork ( );
    /* An error occurred */
    if ( pid2 < 0 ) {
        exit ( EXIT_FAILURE );
    }
    /* Success: Let the parent terminate */
    if ( pid2 > 0 ) {
        exit ( EXIT_SUCCESS );
    }
    /* Set new file permissions */
    if ( !umask ( S_IWGRP | S_IWOTH ) ){
        exit ( EXIT_FAILURE );
    };
    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    char buf_cwd [ MAX_LINE_LENGTH ];
    char buf_cwd2 [ MAX_LINE_LENGTH ];
    char buf_cwd3 [ MAX_LINE_LENGTH ];
    getcwd ( buf_cwd, sizeof ( buf_cwd ) );
    //printf("Now the Current directory is %s\n",buf_cwd);
    chdir ( "/" );
    if ( chdir ( "/" ) == - 1 ) {
        getcwd ( buf_cwd2, sizeof ( buf_cwd2 ) );
        //printf("Now the Current directory is %s\n",buf_cwd2);
        exit ( EXIT_FAILURE );
    };
    getcwd ( buf_cwd3, sizeof ( buf_cwd3 ) );
    //printf("Now the Current directory is %s\n",buf_cwd3);

    /* Close all open file descriptors */
    //for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
    //    //printf("daemonize process close file descriptor:%d !\n",fd);
    //    close(fd);
    //}

    /* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
    stdin  = fopen ( "/dev/null", "r"  );
    stdout = fopen ( "/dev/null", "w+" );
    stderr = fopen ( "/dev/null", "w+" );

    char str [ 256 ];

    /* Try to write PID of daemon to lockfile */
    if ( pid_file_name != NULL )
    {
        pid_fd = open ( pid_file_name, O_RDWR | O_CREAT, 0640 );
        if ( pid_fd < 0 ) {
            /* Can't open lockfile */
            exit ( EXIT_FAILURE );
        }
        if ( lockf ( pid_fd, F_TLOCK, 0 ) < 0 ) {
            /* Can't lock file */
            exit ( EXIT_FAILURE );
        }
        /* Get current PID */
        //printf("Current PID is %d\n",getpid());
        sprintf ( str, "%d\n", getpid ( ) );
        /* Write PID to lockfile */
        write ( pid_fd, str, strlen ( str ) );
    }
    syslog ( LOG_INFO, "segatexd daemonize process succeeded pid is %s", str );
}

/* read pid_file_name and return pid of segatexd.
*/
pid_t read_pid ( )
{
    pid_t pid = 0;
    FILE *fp;
    char linebuf [ MAX_LINE_LENGTH ];
    char *line;

    /* open config file */
    if ( ( fp=fopen(pid_file_name,"r" ) ) == NULL )
    {
        segatex_msg ( LOG_ERR, "cannot open config file (%s): %s", pid_file_name, strerror ( errno ) );
        exit ( EXIT_FAILURE );
    } else
    /* read file and parse lines */
    while ( fgets ( linebuf, sizeof ( linebuf ), fp ) != NULL )
    {
        line = linebuf;
        //segatex_msg(LOG_INFO,"line:%s",line);
    }
    pid = atoi ( line );
    return pid;
}

/* brief Callback function for handling signals.
 * param	sig	identifier of signal
*/
void handle_signal ( int sig )
{
    if ( sig == SIGINT ) {
        /*showing this is SIGINT process*/
        write ( STDOUT_FILENO, msg_sigint, sizeof ( msg_sigint ) - 1 );
        segatex_msg ( LOG_INFO, "%s", msg_sigint );
        running = 3;
        log_stream = stdout;
        /* Reset signal handling to default behavior */
        signal ( SIGINT, SIG_DFL );
    }
    else if (sig == SIGHUP)
    {
        /*showing this is SIGHUP process*/
        write ( STDOUT_FILENO, msg_sighup, sizeof ( msg_sighup ) - 1 );
        segatex_msg ( LOG_INFO, "%s", msg_sighup );
        /*set global int value to 2*/
        SIG_VALUE = 2;
        /* clear configuration */
        cfg_defaults ( segatexd_cfg );
        /* read configfile */
        cfg_read ( fname, segatexd_cfg );
        /*re-daemonize*/
        daemonize ( );
        /* Reset signal handling to default behavior */
        signal ( SIGINT, SIG_DFL );
    }
    else if ( sig == SIGCHLD )
    {
        /*showing this is SIGCHLD process*/
        write ( STDOUT_FILENO, msg_sigchld, sizeof ( msg_sigchld ) - 1 );
    }
}

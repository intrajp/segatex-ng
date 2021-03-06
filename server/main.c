/*
 *  main.c - main file for segatex-ng.
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

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "main.h"
#include "../segatexd.h"
#include "../common/message.c"
#include "daemonize.c"
#include "tcp_server.c"

#define SIG_VALUE 99

/* brief Print help for this application */

void print_help ( void )
{
    printf("\n Usage: %s [OPTIONS]\n\n", app_name);
    printf("  Options:\n");
    //printf("   -p --pid_file  filename   PID file used by daemonized app\n");
    printf("   -t --test_conf filename   Test configuration file\n");
    printf("   -r --reload               Reload the configuration file\n");
    printf("   -s --stop                 Stop segatexd daemon program\n");
    printf("   -d --daemon               Daemonize this application\n");
    printf("   -h --help                 Print this help\n");
    //printf("   -c --conf_file filename   Read configuration from the file\n");
    //printf("   -l --log_file  filename   Write logs to the file\n");
    printf("\n");
}

/*avoid oom killer */

static void avoid_oom_killer ( void )
{
    int oomfd, len, rc;
    char *score = NULL;

    /* New kernels use different technique */	
    if ( ( oomfd = open ( "/proc/self/oom_score_adj",
                O_NOFOLLOW | O_WRONLY ) ) >= 0 ) {
        score = "-1000";
    } else if ( ( oomfd = open("/proc/self/oom_adj",
                O_NOFOLLOW | O_WRONLY ) ) >= 0) {
        score = "-17";
    } else {
        segatex_msg ( LOG_NOTICE, "Cannot open out of memory adjuster" );
        return;
    }

    len = strlen ( score );
    rc = write ( oomfd, score, len );
    //segatex_msg(LOG_INFO,"len:%d",len);
    //segatex_msg(LOG_INFO,"rc:%d",rc);
    if ( rc != len )
        segatex_msg ( LOG_NOTICE, "Unable to adjust out of memory score" );

    close ( oomfd );
}

/*  This function closes the daemon */
 
static int close_daemon ( void )
{
    pid_t pid_x;
    /* Close log file, when it is used. */
    if ( log_stream != stdout ) {
        fclose ( log_stream );
    }

    /* Write system log and close it. */
    segatex_msg ( LOG_INFO, "Stopped %s", app_name );

    pid_x = read_pid ( );
    printf ( "read_pid();%d\n", pid_x );

    /* unlink pid file */
    if ( pid_file_name != NULL )
        unlink ( pid_file_name );

    kill ( pid_x, SIGUSR1 );

    exit ( 0 );
}

/* Main function */

int main ( int argc, char *argv [ ] )
{
    app_name = "segatexd";
    //setting message_mode
    set_sgmessage_mode ( MSG_SYSLOG, DBG_NO ); 

    static struct option long_options [ ] = {
        //{"pid_file", required_argument, 0, 'p'},
        { "test_conf", required_argument, 0, 't' },
        { "reload",    no_argument,       0, 'r' },
        { "stop",      no_argument,       0, 's' },
        { "daemon",    no_argument,       0, 'd' },
        { "help",      no_argument,       0, 'h' },
        { NULL,        0,                 0,  0  }
    };

    int value, option_index = 0, ret;
    char *log_file_name = NULL;
    int start_daemonized = 0;
    int duplicate_value = 0;

    /* Try to process all command line arguments */
    while ( ( value = getopt_long(argc, argv, "trsdh::", long_options, &option_index ) ) != - 1 ) {
        switch ( value ) {
        //   case 'p':
        //        pid_file_name = strdup(optarg);
        //        break;
            case 't':
                cfg_init ( fname );
                break;
            case 'r':
                //printf("duplicate_value:%d\n",duplicate_value);
                if ( duplicate_value == 1 ) {
                    printf("-r, -s and -d could not be selected at the same time\n");
                    return EXIT_FAILURE; 
                }
                duplicate_value = 1;
                cfg_init ( fname );
                handle_signal ( SIGHUP );
                break;
            case 's':
                if ( duplicate_value == 1 ){
                    printf("-r, -s and -d could not be selected at the same time\n");
                    return EXIT_FAILURE; 
                }
                duplicate_value = 1;
                running = 3;
                log_stream = stdout;
                kill ( read_pid ( ), SIGINT );
                break;
            case 'd':
                if ( duplicate_value == 1 ) {
                    printf("-r, -s and -d could not be selected at the same time\n");
                    return EXIT_FAILURE; 
                }
                duplicate_value = 1;
                start_daemonized = 1;
                break;
            case 'h':
                print_help ( );
                return EXIT_SUCCESS;
                case '?':
                print_help ( );
                return EXIT_FAILURE;
            default:
                break;
        }
    }

    /* Tell kernel not to kill us */
    avoid_oom_killer ( );

    printf("SIG_VALUE is %d\n",SIG_VALUE);

    /* Daemon will handle two signals */
    signal ( SIGINT, handle_signal );
    signal ( SIGHUP, handle_signal );

    int i;

    i = is_selinux_enabled ( );

    if ( i == 1 )
        printf("SELinux is enabled.\n");
    else
        printf("SELinux is not enabled.\n");

    /* SIGINT should have running as 3*/
    if ( running != 3 )
    {
        /* Initialize the conf file reading procedure. */
        cfg_init ( fname );

        /* When daemonizing is requested at command line. */
        if ( start_daemonized == 1 ) {
            /* It is also possible to use glibc function deamon()
             * at this point, but it is useful to customize your daemon.
            */
            daemonize ( );
        }

        /* Open system log and write message to it */
        openlog ( argv [ 0 ], LOG_PID | LOG_CONS, LOG_DAEMON );
        segatex_msg ( LOG_INFO, "Started %s", app_name );

        /* Try to open log file to this daemon */
        if ( log_file_name != NULL ) {
            log_stream = fopen ( log_file_name, "a+" );
            if ( log_stream == NULL ) {
                segatex_msg ( LOG_ERR, "Can not open log file: %s, error: %s",
                    log_file_name, strerror ( errno ) );
                log_stream = stdout;
            }
        } else {
            log_stream = stdout;
        }

        /* This global variable can be changed in function handling signal */
        running = 1;
    }

    /* Never ending loop of server */
    //while ( running == 1 ) {
    if ( running == 1 ) {
        /* Debug print */
        ret = fprintf ( log_stream, "Debug: %d\n", counter++ );
        if (ret < 0) {
            segatex_msg ( LOG_ERR, "Can not write to log stream: %s, error: %s",
                ( log_stream == stdout ) ? "stdout" : log_file_name, strerror ( errno ) );
            close_daemon ( );
        }
        ret = fflush ( log_stream );
        if ( ret != 0 ) {
            segatex_msg ( LOG_ERR, "Can not fflush() log stream: %s, error: %s",
                ( log_stream == stdout ) ? "stdout" : log_file_name, strerror ( errno ) );
            close_daemon ( );
        }
        /* TODO: dome something useful here */

        /* Real server should use select() or poll() for waiting at
         * asynchronous event. Note: sleep() is interrupted, when
         * signal is received. 
        */
        segatex_msg ( LOG_INFO, "this is segatexd-server" );

        /* Never ending loop of server */
        tcp_server ( );
        /* end Never ending loop of server */

    }

    close_daemon ( );

    return ( EXIT_SUCCESS );
}

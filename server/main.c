/*
   main.c - main file for segatex-ng.
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
#include "daemonize.c"

/* brief Print help for this application */
void print_help(void)
{
    printf("\n Usage: %s [OPTIONS]\n\n", app_name);
    printf("  Options:\n");
    printf("   -r --reload               Reload the configuration file\n");
    printf("   -d --daemon               Daemonize this application\n");
    printf("   -h --help                 Print this help\n");
    //printf("   -c --conf_file filename   Read configuration from the file\n");
    printf("   -t --test_conf filename   Test configuration file\n");
    //printf("   -l --log_file  filename   Write logs to the file\n");
    printf("   -p --pid_file  filename   PID file used by daemonized app\n");
    printf("\n");
}

/* Main function */
int main(int argc, char *argv[])
{
    //app_name = argv[0];
    app_name = "segatexd";

    static struct option long_options[] = {
        {"test_conf", required_argument, 0, 't'},
        {"reload", no_argument, 0, 'r'},
        {"daemon", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"pid_file", required_argument, 0, 'p'},
        {NULL, 0, 0, 0}
    };

    int value, option_index = 0, ret;
    char *log_file_name = NULL;
    int start_daemonized = 0;
    int duplicate_value = 0;

    /* Try to process all command line arguments */
    while ((value = getopt_long(argc, argv, "ptdrh::", long_options, &option_index)) != -1) {
        switch (value) {
            case 'p':
                pid_file_name = strdup(optarg);
                break;
            case 't':
                cfg_init(fname);
                break;
            case 'd':
                //printf("duplicate_value:%d\n",duplicate_value);
                if ( duplicate_value == 1 ){
                    printf("-r and -d could not be selected at the same time\n");
                    syslog(LOG_ERR, "-r and -d could not be selected at the same time");
                    return EXIT_FAILURE; 
                }
                duplicate_value = 1;
                start_daemonized = 1;
                break;
            case 'r':
                //printf("duplicate_value:%d\n",duplicate_value);
                if ( duplicate_value == 1 ){
                    printf("-r and -d could not be selected at the same time\n");
                    syslog(LOG_ERR, "-r and -d could not be selected at the same time");
                    return EXIT_FAILURE; 
                }
                duplicate_value = 1;
                cfg_init(fname);
                handle_signal(SIGHUP);
                break;
            case 'h':
                print_help();
                return EXIT_SUCCESS;
                case '?':
                print_help();
                return EXIT_FAILURE;
            default:
                break;
        }
    }

    int SIG_VALUE;
    printf("SIG_VALUE is %d\n",SIG_VALUE);

    int i;
    int i2;
    int fd2;

    i = is_selinux_enabled();
    i2 = audit_is_enabled(1);
    if (i = 1)
        printf("SELinux is enabled.\n");
    else
        printf("SELinux is not enabled.\n");
    if (i2 = 1)
        printf("Audit is enabled.\n");
    else if (i2 = 0)
        printf("Audit is not enabled.\n");
    else if (i2 = -1)
        printf("Audit got error.\n");

    /* Initialize the conf file reading procedure. */
    cfg_init(fname);

    /* When daemonizing is requested at command line. */
    if (start_daemonized == 1) {
        /* It is also possible to use glibc function deamon()
         * at this point, but it is useful to customize your daemon. */
        daemonize();
    }

    /* Open system log and write message to it */
    openlog(argv[0], LOG_PID|LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Started %s", app_name);

    /* Daemon will handle two signals */
    signal(SIGINT, handle_signal);
    signal(SIGHUP, handle_signal);

    /* Try to open log file to this daemon */
    if (log_file_name != NULL) {
        log_stream = fopen(log_file_name, "a+");
        if (log_stream == NULL) {
            syslog(LOG_ERR, "Can not open log file: %s, error: %s",
                log_file_name, strerror(errno));
            log_stream = stdout;
        }
    } else {
        log_stream = stdout;
    }

    /* This global variable can be changed in function handling signal */
    running = 1;

    /* Never ending loop of server */

    while (running == 1) {
        /* Debug print */
        ret = fprintf(log_stream, "Debug: %d\n", counter++);
        if (ret < 0) {
            syslog(LOG_ERR, "Can not write to log stream: %s, error: %s",
                (log_stream == stdout) ? "stdout" : log_file_name, strerror(errno));
            break;
        }
        ret = fflush(log_stream);
        if (ret != 0) {
            syslog(LOG_ERR, "Can not fflush() log stream: %s, error: %s",
                (log_stream == stdout) ? "stdout" : log_file_name, strerror(errno));
            break;
        }
        /* TODO: dome something useful here */

        /* Real server should use select() or poll() for waiting at
         * asynchronous event. Note: sleep() is interrupted, when
         * signal is received. */
        syslog(LOG_INFO, "this is segatexd-server");
        sleep(delay);
    }

    /* Close log file, when it is used. */
    if (log_stream != stdout) {
        fclose(log_stream);
    }

    /* Write system log and close it. */
    syslog(LOG_INFO, "Stopped %s", app_name);
    closelog();

    /* Free allocated memory */
    if (pid_file_name != NULL) free(pid_file_name);

    return EXIT_SUCCESS;
}

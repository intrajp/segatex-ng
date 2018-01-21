/*
 *  main.c - main file for segatex-ng agent program.
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

/* transcribed from man getifaddrs */

#include <stdio.h>
#include <stdlib.h>
#include "main.h" 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <linux/if_link.h>

int main ( int argc, char *argv [ ] )
{
    puts ( "-------- This is segatexd-agent --------" );
    const char *app_name_agent = "segatexd-agent";

    struct ifaddrs *ifaddr, *ifa;
    int family, s, n;
    char host [ NI_MAXHOST ];

    int x;
    x = NI_MAXHOST;

    //printf ("NI_MAXHOST:%d\n", x);/* this is 1025 */

    if ( getifaddrs ( &ifaddr ) == - 1 ) {
        perror ( "getifaddrs" );
        exit ( EXIT_FAILURE );
    }

    /* Walk through linked list, maintaining head pointer so we
       can free list later */

    for ( ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++ ) {
        if ( ifa->ifa_addr == NULL )
            continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
           form of the latter for the common families) */

        printf("%-8s %s (%d)\n",
               ifa->ifa_name,
               (family == AF_PACKET) ? "AF_PACKET" :
               (family == AF_INET) ? "AF_INET" :
               (family == AF_INET6) ? "AF_INET6" : "???",
               family);

        /* For an AF_INET* interface address, display the address */

        if ( family == AF_INET || family == AF_INET6 ) {
            s = getnameinfo ( ifa->ifa_addr,
                    ( family == AF_INET ) ? sizeof ( struct sockaddr_in ) :
                                          sizeof ( struct sockaddr_in6 ),
                    host, NI_MAXHOST,
                    NULL, 0, NI_NUMERICHOST);
            if ( s != 0 ) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit ( EXIT_FAILURE );
            }

            printf("\t\taddress: <%s>\n", host);

        }
        else if ( family == AF_PACKET && ifa->ifa_data != NULL ) 
        {
            struct rtnl_link_stats *stats = ifa->ifa_data;

            printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                   "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                   stats->tx_packets, stats->rx_packets,
                   stats->tx_bytes, stats->rx_bytes);
        }
    }

    freeifaddrs ( ifaddr );

    puts ( "-------- End This is segatexd-agent --------" );

    return EXIT_SUCCESS;
}

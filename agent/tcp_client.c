/*
 *  tcp_client.c - functions on tcp client for segatex-ng.
 *  This file contains the contents of segatex-ng.
 *
 *  Copyright (C) 2018 Shintaro Fujiwara 
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

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

/* This function gets message from tcp server */

int tcp_client ( )
{
    struct sockaddr_in server;
    int sockfd = 0;
    char recv_buff [ 4096 ];
    int n;

    /* creating socket */
    sockfd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( sockfd < 0 )
    {
        perror ( "socket" );
        printf ( "%d\n", errno );
        return ( EXIT_FAILURE );
    }

    /* preparing struct for connection destination */
    server.sin_family = AF_INET;
    server.sin_port = htons ( 13579 ); // same as fujiorochi
    /* now i am testing for server on myself */
    server.sin_addr.s_addr = inet_addr ( "127.0.0.1" );

    /* connecting the server */
    if ( connect ( sockfd, ( struct sockaddr * ) &server, sizeof ( server ) ) < 0 )
    {
        printf ("\n Error: Connect Failed \n"); 
        return ( EXIT_FAILURE );
    }

    /* getting something from the server */
    memset ( recv_buff, 0, sizeof ( recv_buff ) );

    //n = read ( sock, buff, sizeof ( recv_buff ) );
    
    while ( ( n = read ( sockfd, recv_buff, sizeof ( recv_buff ) -1 ) ) > 0 )
    {
        recv_buff [ n ] = '\0';
        /* printing out the message from the server */
        if ( fputs ( recv_buff, stdout ) == EOF )
        {
            printf ("\n Error: Fputs error");
        }
        puts ("\n");
    }
    if ( n < 0 )
    {
        printf ("\n Error: Read error");
    }

    /* closing socket */
    close ( sockfd );

    return ( 0 );
}

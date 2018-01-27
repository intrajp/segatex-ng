/*
 *  tcp_server.c - functions on tcp server for segatex-ng.
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

/* This function send message to client */

int tcp_server ( )
{
    int listenfd;
    struct sockaddr_in serv;
    struct sockaddr_in client;
    int len;
    int sock;
    char send_buff [ 1025 ];
    strncpy ( send_buff, "This is segatexd tcp server", 1024 );
    int backlog = 10;

    /* creating socket */
    listenfd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( listenfd < 0 )
    {
        perror ( "socket" );
        printf ( "%d\n", errno );
        return EXIT_FAILURE;
    }

    /* setting socket */
    serv.sin_family = AF_INET;         /* host byte order */
    serv.sin_port = htons ( 13579 );   /* short, network byte order */
    serv.sin_addr.s_addr = htonl ( INADDR_ANY ); /* auto-fill with my IP */
    bzero ( & ( serv.sin_zero ), 8 );            /* zero the rest of the struct */
    bind ( listenfd, ( struct sockaddr * ) &serv, sizeof ( serv ) );

    /* wait for the connection from tcp client */
    if ( listen ( listenfd, backlog ) == -1 )
    {
        printf ( "%d\n", errno );
        return EXIT_FAILURE;
    }

    /* accepting the connection from tcp client */
    len = sizeof ( client );

    while ( 1 )
    {
        sock = accept ( listenfd, ( struct sockaddr *) &client, &len );
        if ( sock < 0 )
        {
            perror ( "socket" );
            printf ( "%d\n", errno );
            return EXIT_FAILURE;
        }

        /* sending message some bytes */
        write ( sock, send_buff, ( int ) strlen ( send_buff ) );

        /* closing tcp session*/
        close ( sock );
        sleep ( 1 );
    }

    /* closing listening socket */
    close ( listenfd );

    return ( 0 );
}

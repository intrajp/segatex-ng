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

#include <stdlib.h>
#include <stdio.h>
#include <string.h> //strlen
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h> //write

//int tcp_client ( )
int main ( int argc, char *argv [ ] )
{
    struct sockaddr_in server;
    int sock;
    //char my_message [ 4096 ], server_reply [ 4096 ];
    char server_reply [ 4096 ];
    const char *my_message = "this is a client message";
    //int n;

    /* creating socket */
    sock = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( sock < 0 )
    {
        perror ( "Creating socket failed. Error" );
        printf ( "%d\n", errno );
        return EXIT_FAILURE;
    }
    puts ("Socket created");

    /* preparing struct for connection destination */
    server.sin_addr.s_addr = inet_addr ( "127.0.0.1" );
    server.sin_family = AF_INET;
    server.sin_port = htons ( 13579 ); // same as fujiorochi

    /* now i am testing for server on myself */
    //inet_pton ( AF_INET, "127.0.0.1", &server.sin_addr.s_addr );

    /* connecting the server */
    if ( connect ( sock, ( struct sockaddr * ) &server, sizeof ( server ) ) < 0 )
    {
        perror ( "connect failed. Error" );
        printf ( "%d\n", errno );
        return EXIT_FAILURE;
    }
    puts ("Connect done");

    /* getting something from the server */
    /* keep communication with the server */
    //while ( 1 )
    //{
        //printf ( "Enter message : " );
        //scanf ( "%s", my_message);
  
        //sending some data 
        if ( send ( sock, my_message, strlen ( my_message ), 0 ) < 0 )
        {
            puts ("Send failed");
            printf ( "%d\n", errno );
            return EXIT_FAILURE;
        }

        //receive a reply from the server
        if ( recv ( sock, server_reply, 4096, 0 ) < 0 )
        {
            puts ( "recv failed" );
            //break;
        }
        puts ( "Server reply :" );
        puts (server_reply);
    //}
    //memset ( buff, 0, sizeof ( buff ) );
    //n = read ( sock, buff, sizeof ( buff ) );

    //printf( "getting message from server:%d, %s\n", n, buff );

    /* closing socket */
    close ( sock );

    return ( 0 );
}

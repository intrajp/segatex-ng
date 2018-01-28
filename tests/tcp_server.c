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

#include <stdlib.h>
#include <stdio.h>
#include <string.h> //strlen
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h> //write

//int tcp_server ( )
int main ( int argc, char *argv [ ] )
{
    int sock0, client_sock;//socket file discriptor
    int listen_n, len, read_size;
    struct sockaddr_in server;
    struct sockaddr_in client;
    char client_message [ 4096 ];
    const char *server_message = "This is segatexd-tcp-server.";
    //strncpy ( client_message, "This is segatexd tcp-server", 2000 );

    /* creating socket */
    sock0 = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( sock0 < 0 )
    {
        perror ( "Creating socket failed. Error" );
        printf ( "%d\n", errno );
        return EXIT_FAILURE;
    }
    printf ("Socket created file discriptor:%d\n",sock0);

    /* setting socket */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons ( 13579 );
    if ( bind ( sock0, ( struct sockaddr * ) &server, sizeof ( server ) ) < 0 )
    {
        perror ( "bind failed. Error" );
    }
    puts ("bind done");

    /* wait for the connection from tcp client */
    listen_n = listen ( sock0, 128 );
    printf ( "listen_n is :%d\n", listen_n);

    /* accepting the connection from tcp client */
    puts ("Waiting for incoming connections...");
    len = sizeof ( client );

    client_sock = accept ( sock0, ( struct sockaddr * ) &client, ( socklen_t * ) &len );

    if ( client_sock < 0 )
    {
        perror ( "accept failed" );
        printf ( "%d\n", errno );
        return EXIT_FAILURE;
    }
    puts ("Connection accepted");

    /* sending something some bytes */
    //write ( client_sock, server_message, strlen ( server_message ) );

    if ( ( read_size = recv ( client_sock, client_message, 4096, 0 ) ) > 0 )
    {
        write ( client_sock, server_message, strlen ( server_message ) );
        close ( sock0 );
    }

    /* Recieve a message from client */
/*
    while ( ( read_size = recv ( client_sock, client_message, 4096, 0 ) ) > 0 )
    {
        //Send the message back to client
        write ( client_sock, client_message, strlen ( client_message ) );
    }
*/
    /* closing tcp session*/
    if ( read_size == 0 )
    {
        puts ( "Client disconnected" );
        fflush ( stdout );
    } 
    else if ( read_size == -1 )
    {
        perror ( "recv failed" );
    }

    close ( client_sock );

    /* closing listening socket */
    close ( sock0 );
    puts ( "Closed file discriptor" );

    return ( 0 );
}

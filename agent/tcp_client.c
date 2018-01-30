/*
 *  tcp_client.c - functions on tcp client for segatex-ng.
 *  This file contains the contents of segatex-ng.
 *
 *  Copyright (C) 2017-2018 Shintaro Fujiwara
 *
 *  Many thanks to
 *  fac(ulty).ksu.edu.sa/mdahshan/CEN463FA09/07-file_transfer_ex.pdf
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

#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> //struct sockaddr_in, inet_ntop(), inet_ntoa(), inet_aton()
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h> //printf(), fprintf(), perror() and standard I/O
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strlen(), strcmp()
#include <unistd.h> //close(), write(), read()
#include <fcntl.h> //open()
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include "../segatexd.h"

/* This function gets message from tcp server */

#define MAX_RECV_BUFF 256  
#define MAX_SEND_BUFF 256  
#define PORT_NUMBER 13579  
#define SERVER_ADDRESS "127.0.0.1" 

char file_name_receive [ MAX_RECV_BUFF ] = "/tmp/";
char real_file_name [ MAX_RECV_BUFF + 1 ];

/* This function receives file name from the tcp server */

int recv_file_name ( int sock_fd )
{
    char recvBuff [ MAX_RECV_BUFF ];
    memset ( recvBuff, '\0' ,sizeof ( recvBuff ) );
 
    //segatex_msg ( LOG_NOTICE, "connected to:%s:%d ..\n", SERVER_ADDRESS, PORT_NUMBER );
    printf ( "connected to:%s:%d ..\n", SERVER_ADDRESS, PORT_NUMBER );

    //receive a reply from the server
    if ( recv ( sock_fd, recvBuff, MAX_RECV_BUFF, 0 ) < 0 )
    {
        puts ( "recv failed" );
    }
    //now copy the result to the string
    strncpy ( real_file_name, recvBuff, MAX_RECV_BUFF ); 

    return ( 0 );
}

/* This function receives file from tcp server*/

int recv_file ( int sock, char *real_file_name )
{
    /* creating file name */
    char f_t [ 40 ];
    memset ( f_t, '\0', 40 );

    printf ( "file_name:%s\n",real_file_name );
    strncat ( file_name_receive, real_file_name, 250 );
    printf ( "file_name_receive:%s\n",file_name_receive );
 
    char send_str [ MAX_SEND_BUFF ]; /* message to be sent to server*/

    int f; /* file handle for receiving file*/
    ssize_t sent_bytes, rcvd_bytes, rcvd_file_size;
    int recv_count; /* count of recv() calls*/

    char recv_str [ MAX_RECV_BUFF ]; /* buffer to hold received data */

    size_t send_strlen; /* length of transmitted string */

    sprintf ( send_str, "%s\n", file_name_receive ); /* add CR/LF (new line) */

    send_strlen = strlen ( send_str ); /* length of message to be transmitted */

    if ( ( sent_bytes = send ( sock, file_name_receive, send_strlen, 0 ) ) < 0 ) {
        perror ( "send error" );
        return ( EXIT_FAILURE );
    }

    /* attempt to create file to save received data. 0644 = rw-r--r-- */
    if ( ( f = open ( file_name_receive, O_WRONLY | O_CREAT, 0644 ) ) < 0 )
    {
        perror ( "error creating file" );
        return ( EXIT_FAILURE );
    }

    recv_count = 0; /* number of recv() calls required to receive the file */
    rcvd_file_size = 0; /* size of received file */

    /* continue receiving until ? (data or close) */
    while ( ( rcvd_bytes = recv ( sock, recv_str, MAX_RECV_BUFF, 0 ) ) > 0 )
    {
        recv_count++;
        rcvd_file_size += rcvd_bytes;

        if ( write ( f, recv_str, rcvd_bytes ) < 0 )
        {
            perror( "error writing to file" );
            return ( EXIT_FAILURE );
        }

        /* break if rcvd_bytes is less than MAX_RECV_BUFF, meaning the end */
        if ( rcvd_bytes < MAX_RECV_BUFF )
           break; 
    }

    close ( f ); /* close file*/

    //segatex_msg ( LOG_NOTICE, "Client Received: %d bytes in %d recv(s)\n", rcvd_file_size,
    printf ( "Client Received: %ld bytes in %d recv(s)\n", rcvd_file_size,
            recv_count);

    return rcvd_file_size;
}

/* This function do job as it says */

int tcp_client ( void )
{
    int sock_fd = 0;
    struct sockaddr_in srv_addr;
    char recv_buff [ 4096 ];

    /* creating socket */
    sock_fd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( sock_fd < 0 )
    {
        perror ( "socket" );
        segatex_msg ( LOG_NOTICE, "%d\n", errno );
        return ( EXIT_FAILURE );
    }

    /* getting something from the server */
    memset ( recv_buff, '\0', sizeof ( recv_buff ) ); /* fill '\0' char recv_buff */
    memset ( &srv_addr, 0, sizeof ( srv_addr ) ); /* zero-fill srv_addr struct */

    /* preparing struct for connection destination */
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons ( PORT_NUMBER ); // same as fujiorochi
    /* now i am testing for server on myself */
    srv_addr.sin_addr.s_addr = inet_addr ( SERVER_ADDRESS );

    /* connecting the server */
    if ( connect ( sock_fd, ( struct sockaddr * ) &srv_addr, sizeof ( srv_addr ) ) < 0 )
    {
        //segatex_msg ( LOG_NOTICE, "\n Error: Connect Failed \n" ); 
        printf("\n Error: Connect Failed on file recv\n"); 
        return ( EXIT_FAILURE );
    }

    //segatex_msg ( LOG_NOTICE, "connected to:%s:%d ..\n", SERVER_ADDRESS, PORT_NUMBER );
    printf ( "connected for file receive to:%s:%d ..\n", SERVER_ADDRESS, PORT_NUMBER );

    /* receiving file */
    /* first, getting file name from server */
    if (recv_file_name ( sock_fd ) == 0 )
    {
        printf ("real_file_name:%s\n", real_file_name);
    }
    else
    {
        puts("no file name received");
        return 0;
    }

    /* next, getting contents from the server */
    recv_file ( sock_fd, real_file_name);

    /* close socket*/
    if(close(sock_fd) < 0)
    {
        perror ( "socket close error" );
        exit ( EXIT_FAILURE );
    }

    return ( 0 );
}

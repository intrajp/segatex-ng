/*
 *  tcp_server.c - functions on tcp server for segatex-ng.
 *  This file contains the contents of segatex-ng.
 *
 *  Copyright (C) 2018 Shintaro Fujiwara 
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

#include <sys/socket.h> // socket(), bind(), listen(), accept(), socklen_t
#include <netinet/in.h>
#include <arpa/inet.h> //struct sockaddr_in, inet_ntop(), inet_ntoa(), inet_aton()
#include <stdio.h> //printf(), fprintf(), perror() and standard I/O
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset(), strlen(), strcmp()
#include <unistd.h> // close(), write(), read()
#include <fcntl.h> //open()
#include <errno.h>
#include <sys/types.h>

#define MAX_RECV_BUFF 256  
#define MAX_SEND_BUFF 256  
#define PORT_NUMBER 13579  
#define FILE_NAME_SEND "/usr/share/segatexd/segatexd_tcp_send_data.txt" 

/*  This function get file_name and store it to char array,
 *  do nothing, not used for the future use only
*/

void get_file_name ( int sock, char *file_name )
{
    char recv_str [ MAX_RECV_BUFF ]; /* to store received string */
    memset ( recv_str, '\0', MAX_RECV_BUFF );
    ssize_t rcvd_bytes; /* bytes received from socket */
    /* read name of requested file from socket */
    if ( ( rcvd_bytes = recv ( sock, recv_str, MAX_RECV_BUFF, 0 ) ) < 0 )
    {
        perror ( "recv error" );
        return;
    }
    sscanf ( recv_str, "%s\n", file_name ); /* discard CR/LF */
}

/* This function send file to client*/

int send_file ( int sock, const char * file_name )
{
    int sent_count;
    ssize_t read_bytes,
            sent_bytes,
            sent_file_size;
    char send_buff [ MAX_SEND_BUFF ];
    char *errmsg_notfound = "File not found\n";
    int f;
    sent_count = 0;
    sent_file_size = 0;

    memset ( send_buff, '\0', sizeof ( send_buff ) ); // fill up '\0' to char array

    /* attempt to open requested file for reading */
    if ( ( f = open ( file_name, O_RDONLY ) ) < 0 ) /* can't open requested file */
    {
        perror ( file_name );
        if ( ( sent_bytes = send ( sock, errmsg_notfound,
                strlen ( errmsg_notfound ), 0 ) ) < 0 )
        {
            perror ( "send error" );
            return EXIT_FAILURE;
        }
    }
    else /* open file successful */
    {
        segatex_msg ( LOG_NOTICE, "Sending file: %s\n", file_name );
        while ( ( read_bytes = read ( f, send_buff, MAX_RECV_BUFF ) ) > 0 )
        {
            if ( ( sent_bytes = send ( sock, send_buff, read_bytes, 0 ) )
                    < read_bytes )
            {
                perror ( "send error" );
                return EXIT_FAILURE;
            }
            sent_count++;
            sent_file_size += sent_bytes;
        }
        close ( f );
    }

    segatex_msg (LOG_NOTICE, "Done with this client. Sent %ld bytes in %d send(s)\n\n",
            sent_file_size, sent_count);

    return sent_count;
}

/* This function send message or file to client */

int tcp_server ( )
{
    segatex_msg ( LOG_NOTICE, "Hey, i am tcp server !");

    int listen_fd;
    struct sockaddr_in srv_addr;
    struct sockaddr_in cli_addr;
    int len;
    int conn_fd;
    char send_buff [ 1025 ];
    char print_addr [ 30 ];
    strncpy ( send_buff, "This is segatexd tcp server", 1024 );
    int backlog = 10;

    /* setting socket */
    memset ( &srv_addr, 0, sizeof ( srv_addr ) ); /* zero-fill srv_addr struct */
    memset ( &cli_addr, 0, sizeof ( cli_addr ) ); /* zero-fill cli_addr struct */

    srv_addr.sin_family = AF_INET;         /* host byte order */
    srv_addr.sin_addr.s_addr = htonl ( INADDR_ANY ); /* auto-fill with my IP */
    srv_addr.sin_port = htons ( PORT_NUMBER );   /* short, network byte order */
    bzero ( & ( srv_addr.sin_zero ), 8 );            /* zero the rest of the struct */

    /* creating socket */
    listen_fd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( listen_fd < 0 )
    {
        perror ( "socket" );
        segatex_msg ( LOG_NOTICE, "%d\n", errno );
        return EXIT_FAILURE;
    }

    /* bind to created socket */
    if ( bind ( listen_fd, ( struct sockaddr * ) &srv_addr, sizeof ( srv_addr ) ) < 0 )
    {
        perror ( "bind error" );
        segatex_msg ( LOG_NOTICE, "%d\n", errno );
        exit ( EXIT_FAILURE );
    }

    /* wait for the connection from tcp client */
    segatex_msg ( LOG_NOTICE, "Listening on port number %d ...\n", ntohs ( srv_addr.sin_port ) );
    if ( listen ( listen_fd, backlog ) == -1 )
    {
        perror ( "listen error" );
        segatex_msg ( LOG_NOTICE, "%d\n", errno );
        return EXIT_FAILURE;
    }

    while ( 1 )
    {
        /* accepting the connection from tcp client */
        len = sizeof ( cli_addr );

        if ( ( conn_fd = accept ( listen_fd, ( struct sockaddr *) &cli_addr, &len ) ) < 0 )
        {
            perror ( "accept error " );
            segatex_msg ( LOG_NOTICE, "%d\n", errno );
            /* exit from the while loop */
            break;
        }

        /* convert numeric IP to readable format for displaying */
        inet_ntop(AF_INET, &(cli_addr.sin_addr), print_addr, INET_ADDRSTRLEN);
        segatex_msg ( LOG_NOTICE, "Clinet connected from %s:%d\n",
                print_addr, ntohs(cli_addr.sin_port) );

        /* sending message some bytes */
        //write ( conn_fd, send_buff, ( int ) strlen ( send_buff ) );
        //get_file_name ( conn_fd, FILE_NAME_SEND );
        send_file ( conn_fd, FILE_NAME_SEND );
    }

    /* closing tcp session*/
    if ( close ( conn_fd ) < 0 )
    {
        perror ( "socket conn_fd close error" );
        exit ( EXIT_FAILURE );
    }

    /* closing listening socket */
    close ( listen_fd );
    {
        perror( "socket listen_fd close error" );
        exit( EXIT_FAILURE );
    }

    return ( 0 );
}

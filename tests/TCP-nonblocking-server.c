/***************************************************************************/
/*                                                                         */
/* Server program which wait for the client to connect and reads the data  */
/*     using non-blocking socket.                                          */
/*                                                                         */
/* The message is sent after 5 seconds of wait (from the client)           */
/*                                                                         */
/*                                                                         */
/* based on Beej's program - look in the simple TCP server for further doc.*/
/*                                                                         */
/*                                                                         */
/***************************************************************************/
 
    #include <stdio.h> 
    #include <stdlib.h> 
    #include <errno.h> 
    #include <string.h> 
    #include <sys/types.h> 
    #include <netinet/in.h> 
    #include <sys/socket.h> 
    #include <sys/wait.h> 
    #include <fcntl.h> /* Added for the nonblocking socket */
    #include <sys/time.h>
    #include <unistd.h>


    #define MYPORT 3456    /* the port users will be connecting to */

    #define BACKLOG 10     /* how many pending connections queue will hold */

    main()
    {
        int 			sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd */
        struct 	sockaddr_in 	my_addr;    /* my address information */
        struct 	sockaddr_in 	their_addr; /* connector's address information */
        int 			sin_size;
	char			string_read[255];
	int 			n,i;
	struct	timeval		tv_before, tv_after;
	struct	timezone	tz;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        my_addr.sin_family = AF_INET;         /* host byte order */
        my_addr.sin_port = htons(MYPORT);     /* short, network byte order */
        my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
        bzero(&(my_addr.sin_zero), 8);        /* zero the rest of the struct */

        if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) \
                                                                      == -1) {
            perror("bind");
            exit(1);
        }

        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            exit(1);
        }

        while(1) {  /* main accept() loop */
            sin_size = sizeof(struct sockaddr_in);
            if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
                                                          &sin_size)) == -1) {
                perror("accept");
                continue;
            }
            printf("server: got connection from %s\n", \
                                               inet_ntoa(their_addr.sin_addr));
	    gettimeofday(&tv_before,&tz);
/***********************************************************************************/
/* The non blocking area							   */
/***********************************************************************************/
    	    fcntl(new_fd, F_SETFL, O_NONBLOCK);
	    n=recv(new_fd,string_read,sizeof(string_read),0);
	    i=2;
	    while(n < 1){
	    	n=recv(new_fd,string_read,sizeof(string_read),0);
		if (n < 1) 
			perror("recv - non blocking \n");
	    	printf("Round %d, and the data read size is: n=%d \n",i,n);
		i++;
	    }
	    gettimeofday(&tv_after,&tz);
	    printf("Before reading: Time=%ld, Sec=%ld\n",tv_before.tv_sec, tv_before.tv_usec);
            printf("After reading : Time=%ld, Sec=%ld\n",tv_after.tv_sec, tv_after.tv_usec);

	    string_read[n] = '\0';
	    printf("The string is: %s \n",string_read);
	    
            if (send(new_fd, "Hello, world!\n", 14, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
            close(new_fd);  /* parent doesn't need this */

            while(waitpid(-1,NULL,WNOHANG) > 0); /* clean up child processes */
        }
    }


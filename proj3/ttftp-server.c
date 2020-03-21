/*
** name: ttftp-server.c
**
** author:
** created:
** last modified:
**
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<assert.h>
#include<unistd.h>

#include "ttftp.h"


// void check(int val,char *error_msg);
// void checknull(void * ptr);

int  ttftp_server( int listen_port, int is_noloop ) {

	int sockfd_l;
	int sockfd_s ;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct addrinfo hints;
	struct addrinfo *addrs;
	int block_count ;
	
	/*
	 * create a socket to listen for RRQ
	 */
	//allocate space for port #
	char *l_port = malloc(2);
	//convert port to string
	sprintf(l_port,"%d",listen_port);
	//get address of server
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	//check((getaddrinfo(NULL,l_port,&hints,&addrs)),"failed to resolve hostname");
	if(getaddrinfo(NULL,l_port,&hints,&addrs) != 0)
	{
		perror("failed to resolve hostname");
		exit(1);
	}
	//open socket to listen
	//check((sockfd_l = socket(addrs->ai_family,addrs->ai_socktype,addrs->ai_protocol)),"failed to create socket");
	if((sockfd_l = socket(addrs->ai_family,addrs->ai_socktype,addrs->ai_protocol)) < 0)
	{
		perror("failed to create socket");
		exit(1);
	}

	if((bind(sockfd_l,addrs->ai_addr,addrs->ai_addrlen)) < 0)
	{
		perror("failed to bind socket");
		exit(1);
	}

	do {
	
		/*
		 * for each RRQ 
		 */
		char buffer[TFTP_DATALEN];
		//hold # of bytes received
		int recvbytes = 0;
		//get bytes
		//check((recvbytes = recvfrom(sockfd_l,buffer,TFTP_DATALEN-1,0,(struct sockaddr*)&their_addr,&socksize)),"error receiving bytes");

		if((recvbytes = recvfrom(sockfd_l,buffer,TFTP_DATALEN-1,0,(struct sockaddr*)&their_addr,&socksize)) < 0)	
		{
			perror("error recieving bytes");
			exit(1);
		}
		/*
		 * TODO: parse request and open file
		 */
		TftpReq *recv_rrq_packet = (TftpReq*)buffer;
		write(1,recv_rrq_packet->filename_and_mode,recvbytes);
		/*
		 * create a sock for the data packets
		 */	 
		//check((sockfd_s = socket(AF_INET,SOCK_DGRAM,0)),"failed to create socket");
		//TftpData *data_packet = malloc(sizeof(TftpData));
		block_count = 0 ;
		while (block_count) { 

			/*
			 * TODO: read from file
			 */
			 
			/*
			 * send data packet
			 */
			//data_packet->data = buffer;	
			//check((sentbytes=sendto(sockfd_s,data_packet,sizeof(data_packet))) 
			/*
			 * wait for acknowledgement & DO NOT SEND PCKT IF DUP ACK
			 */
			 
			block_count++ ;
		}
	
	} while (!is_noloop) ;
	return 0 ;
}


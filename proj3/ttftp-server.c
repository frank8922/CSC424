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

#define TRUE 1
#define FALSE 0


int  ttftp_server( int listen_port, int is_noloop ) {

	int sockfd_l;
	int sockfd_s ;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct addrinfo hints;
	struct addrinfo *addrs;
	int block_count ;
	TftpError *error_packet;
	
	/*
	 * create a socket to listen for RRQ
	 */
	//allocate space for port #
	char *l_port = malloc(2);
	//convert port to string
	sprintf(l_port,"%d",listen_port);
	//get address of server
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	check((getaddrinfo(NULL,l_port,&hints,&addrs) != 0),"failed to resolve");
	//open socket to listen
	check((sockfd_l = socket(addrs->ai_family,addrs->ai_socktype,addrs->ai_protocol)),"failed to create socket");
	//bind to the socket
	check((bind(sockfd_l,addrs->ai_addr,addrs->ai_addrlen)),"failed to bind socket");

	do {
	
		/*
		 * for each RRQ 
		 */
		void *buffer = malloc(TFTP_DATALEN);
		//hold # of bytes received
		int recvbytes = 0;
		//get bytes
		check((recvbytes = recvfrom(sockfd_l,buffer,TFTP_DATALEN-1,0,(struct sockaddr*)&their_addr,&socksize)),"error recieving bytes");	
    	printf("number of bytes recv %d\n",recvbytes);
		/*
		 * TODO: parse request and open file
		 */
		TftpReq *recv_rrq_packet = NULL;
		FILE * file = NULL;
		char opcode = *((char*)buffer);
		if(opcode == TFTP_RRQ)
		{
			recv_rrq_packet = (TftpReq*)buffer;
			printf("%s\n",recv_rrq_packet->filename_and_mode);
			file = openFile(recv_rrq_packet->filename_and_mode,sockfd_l,&their_addr);
		}
		
		/*
		* create a sock for the data packets
		*/	 
		check((sockfd_s = socket(AF_INET,SOCK_DGRAM,0)),"failed to create socket");
		//allocate space for data packet
		int bytes_read = 0;

		block_count = 1 ;
		while (block_count) 
		{ 
			/*
			* TODO: read from file
			*/
			TftpData *data_packet = createDataPacket(file,block_count);
			/*
			* send data packet
			*/
			if(!feof(file))
			{
				printf("%s\n",data_packet->data);
				sendDataPacket(sockfd_s,their_addr,data_packet);
				break;
			}
			/*
			* wait for acknowledgement & DO NOT SEND PCKT IF DUP ACK
			*/
			check((recvfrom()),"error receiving bytes");
			TftpAck *recv_ack_packet = (TftpAck*)buffer;

			block_count++ ;
		}
		
	
	} while (!is_noloop) ;
	return 0 ;
}


void check(int val, char *error_msg)
{
  if(val < 0)
  {
    perror(error_msg);
  }
}


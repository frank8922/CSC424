/*
** name: ttftp-client.c
**
** author: 
** created:
** last modified:
**
** from template created 31 jan 2015 by bjr
**
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<assert.h>
#include<unistd.h>

#include "ttftp.h"

#define h_addr h_addr_list[0]

int ttftp_client( char * to_host, int to_port, char * file ) {
	int block_count ; 
	void *buffer = malloc(TFTP_DATALEN);
	struct sockaddr_in from_addr;
	socklen_t socksize = sizeof(struct sockaddr_in);
	TftpError *error_packet = NULL;
	TftpData *data_packet = NULL;
	
	/*
	 * create a socket to send
	 */
	int recvbytes = 0, 
		sentbytes = 0, 
		 sockfd_s = 0;

	char * port;
	struct addrinfo hints;
	struct addrinfo *addrs;
	
	//zero out memory of hints
	memset(&hints,0,sizeof(hints));
	//set family
	hints.ai_family = AF_INET;
	//set socktype
	hints.ai_socktype = SOCK_DGRAM;
	//use available ip
	hints.ai_flags = AI_PASSIVE;

	//allocate memory for port
	port = malloc(2);
	//convert port to string for getaddrinfo
	sprintf(port,"%d",to_port);
	//get hostname of sender
	check((getaddrinfo(to_host,port,&hints,&addrs) !=0),"failed to resolve hostname");
	//create socket
	check((sockfd_s = socket(addrs->ai_family,addrs->ai_socktype,addrs->ai_protocol)),"failed to create socket");
	/*
	 * send RRQ
	 */
	TftpReq *rrq = createRRQ(file);
	sentbytes = sendRRQ(rrq,sockfd_s,addrs->ai_addr);
	printf("sent %d bytes\n",sentbytes);
	block_count = 1 ; /* value expected */
	while ( block_count ) {
		/*
		 * read a DATA packet
		 */
		check((recvbytes = recvfrom(sockfd_s,buffer,TFTP_DATALEN-1,0,(struct sockaddr*)&from_addr,&socksize)),"error receiving bytes");	
		printf("number of bytes recv %d\n",recvbytes);
		char opcode = *((char*)buffer);
		switch(opcode)
		{
			case '5':
			error_packet = (TftpError*)buffer;
			printf("error: %s\n",error_packet->error_msg);
			break;
			case '3':
			data_packet = (TftpData*)buffer;
			printf("data: %s\n",data_packet->data);
			printf("\n");
			//send ack packet
			sentbytes = sendAckPacket(block_count,sockfd_s,&from_addr);
			printf("sent ack %d of %d\n",block_count,sentbytes);
			//increment block count
			block_count++ ;
			break;
		}
		
		/* check if more blocks expected, else 
		 * set block_count = 0 ;
		 */
		if(recvbytes < TFTP_DATALEN-1)
		{
			block_count = 0;
			break;
		}
	}
	return 0 ;
}

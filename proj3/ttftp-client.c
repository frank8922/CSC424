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
//#define IS_NULL(X) checknull((X))

// void check(int val,char *error_msg);
// void checknull(void * ptr);

int ttftp_client( char * to_host, int to_port, char * file ) {
	int block_count ; 
	
	/*
	 * create a socket to send
	 */
	int sockfd_s = 0;
	int sockfd_l = 0;
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
	sendRRQ(rrq,sockfd_s,addrs->ai_addr);
	
	block_count = 1 ; /* value expected */
	while ( block_count ) {
		struct sockaddr from_addr;
		socklen_t socksize = sizeof(struct sockaddr);
		void *buffer = malloc(TFTP_DATALEN);
		/*
		 * read a DATA packet
		 */
		//hold # of bytes received
		int recvbytes = 0;
		//get bytes
		check((recvbytes = recvfrom(sockfd_s,buffer,TFTP_DATALEN-1,0,&from_addr,&socksize)),"error receiving bytes");	
		TftpError *error_packet = NULL;
		TftpData *data_packet = NULL;
		TftpAck *ack_packet = NULL;
		//check packet opcode and cast packet to appropriate type
		char opcode = *((char*)buffer);
		switch(opcode)
		{
			case '5':
			error_packet = (TftpError*)buffer;
			printf("error: %s\n",error_packet->error_msg);
			break;
			case '3':
			data_packet = (TftpData*)buffer;//allocate space for ack packet
			printf("data: %s\n",data_packet->data);
			ack_packet = malloc(sizeof(TftpAck));
			memset(ack_packet,0,sizeof(TftpAck));
			sprintf(ack_packet->opcode,"%d",TFTP_ACK);
			sprintf(ack_packet->block_num,"%d",block_count);
			//send ack packet
			check((sentbytes = sendto(sockfd_s,ack_packet,sizeof(TftpAck),0,addrs->ai_addr,addrs->ai_addrlen)),"failed to send bytes");
			//free ack packet
			free(ack_packet);
			break;
			case '4':
			ack_packet = (TftpAck*)buffer;
			printf("block num: %s\n",ack_packet->block_num);
			break;
		}

		//check if packet is null
		if(buffer == NULL)
		{
			fprintf(stderr,"NULL POINTER (%s:%d)\n",__FILE__,__LINE__);
			exit(1);
		}

		/*
		 * write bytes to stdout
		 */

		/*
		 * send an ACK
		 */
		
		//increment block count
		block_count ++ ;
		

		/* check if more blocks expected, else 
		 * set block_count = 0 ;
		 */

	}
	return 0 ;
}
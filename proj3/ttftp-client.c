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
	int recvbytes = 0, 
		sentbytes = 0, 
		block_count;
	void *buffer = malloc(MAXMSGLEN);
	struct sockaddr_in from_addr,their_addr;
	socklen_t socksize = sizeof(struct sockaddr_in);
	TftpError *error_packet = NULL;
	TftpData *data_packet = NULL;
	
	/*
	 * create a socket to send
	 */
	int sockfd = 0;

	short opcode;
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
	//create socket to send packets
	check((sockfd = socket(addrs->ai_family,addrs->ai_socktype,addrs->ai_protocol)),"failed to create socket");
	/*
	 * send RRQ
	 */
	TftpReq *rrq = createRRQ(file);
	sentbytes = sendRRQ(rrq,sockfd,addrs->ai_addr);
	freeaddrinfo(addrs);

	//create a socket listen to packets

	block_count = 1;
	while ( block_count ) {
		/*
		 * read a DATA packet
		 */
		check((recvbytes = recvfrom(sockfd,buffer,MAXMSGLEN-1,0,(struct sockaddr*)&from_addr,&socksize)),"error receiving bytes");	
		opcode = *((short*)buffer);
		switch(opcode)
		{
			case TFTP_ERR:
				error_packet = (TftpError*)buffer;
				printf("error: %s\n",error_packet->error_msg);
			break;
      
			case TFTP_DATA:
				data_packet = (TftpData*)buffer;
				if((data_packet->data==NULL) || recvbytes < 4)
				{
				//recieved an empty packet
				sentbytes = sendAckPacket(block_count,sockfd,&from_addr);
				break;
				}
				fwrite(data_packet->data,1,recvbytes-4,stdout);
				printf("\n");
				//send ack packet
				sentbytes = sendAckPacket(block_count,sockfd,&from_addr);
				//increment block count
				block_count++ ;
			break;
		}
		
		/* check if more blocks expected, else 
		 * set block_count = 0 ;
		 */
		if(recvbytes-4 < TFTP_DATALEN-1)
		{
			block_count = 0;
			break;
		}
	}
	close(sockfd);
	return 0 ;
}

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
	int sock;
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
	//check((getaddrinfo(to_host,port,&hints,&addrs)),"failed to resolve host");
	if(getaddrinfo(to_host,port,&hints,&addrs) !=0)
	{
		perror("failed to resolve hostname");
		exit(1);
	}
	//create socket
	// check((sock = socket(addrs->ai_family,addrs->ai_socktype,addrs->ai_protocol)),"failed to create socket");
	if((sock = socket(addrs->ai_family,addrs->ai_socktype,addrs->ai_protocol)) < 0)
	{
		perror("failed to create socket");
		exit(1);
	}
	//bind socket
	// check((bind(sock,addrs->ai_addr,addrs->ai_addrlen)),"failed to bind socket");
	/*if((bind(sock,addrs->ai_addr,addrs->ai_addrlen)) < 0)*/
	/*{*/
		/*perror("failed to bind socket");*/
		/*exit(1);*/
	/*}*/
	/*
	 * send RRQ
	 */
	//holds # of bytes sent
	int sentbytes = 0;
	//create new rrq packet
	TftpReq *rrq_packet = malloc(sizeof(TftpReq));
	//set opcode
	sprintf(rrq_packet->opcode,"%d",TFTP_RRQ);
	//check if filename is null
	// IS_NULL(file);
	if(file == NULL)
	{
		fprintf(stderr,"NULL POINTER (%s:%d)\n",__FILE__,__LINE__);
		exit(1);
	}
	//allocate space for filename and mode
	int len = strlen(OCTET_STRING)+1+strlen(file)+1;
	char *mode_and_fname = malloc(len);
	//store filename and mode
	sprintf(mode_and_fname,"%s",file);
	sprintf((mode_and_fname)+strlen(file)+1,"%s",OCTET_STRING);
	//set filename and mode
	rrq_packet->filename_and_mode = mode_and_fname;
	//send RRQ
	// check((sentbytes=sendto(sock,rrq_packet,sizeof(TftpReq) + len,0,addrs->ai_addr,addrs->ai_addrlen)),"failed to send bytes");
	if((sentbytes=sendto(sock,rrq_packet,sizeof(rrq_packet) + len,0,addrs->ai_addr,addrs->ai_addrlen)) < 0)
	{
		perror("failed to send bytes");
		exit(1);
	}
	//free packet
	free(rrq_packet);

	
	block_count = 0 ; /* value expected */
	while ( block_count ) {
		struct sockaddr from_addr;
		socklen_t socksize = sizeof(struct sockaddr);
		char buffer[TFTP_DATALEN] = {0};
		/*
		 * read a DATA packet
		 */
		//hold # of bytes received
		int recvbytes = 0;
		//get bytes
		// check((recvbytes = recvfrom(sock,buffer,TFTP_DATALEN-1,0,&from_addr,&socksize)),"error receiving bytes");
		if((recvbytes = recvfrom(sock,buffer,TFTP_DATALEN-1,0,&from_addr,&socksize)) < 0)	
		{
			perror("error recieving bytes");
			exit(1);
		}
		//allocate space for packet
		TftpData *data_packet = malloc(sizeof(TftpData)); //SIZE VARIES
		//check if packet is null
		if(buffer == NULL)
		{
			fprintf(stderr,"NULL POINTER (%s:%d)\n",__FILE__,__LINE__);
			exit(1);
		}
		//set data packet to buffer
		data_packet->data = buffer;
		/*
		 * write bytes to stdout
		 */
		write(1,data_packet->data,recvbytes);
		//free data packet
		free(data_packet);
		/*
		 * send an ACK
		 */
		//allocate space for ack packet
		TftpAck *ack_packet = malloc(sizeof(TftpAck));
		sprintf(ack_packet->opcode,"%d",TFTP_ACK);
		sprintf(ack_packet->block_num,"%d",block_count);
		//send ack packet
		if((sentbytes = sendto(sock,ack_packet,sizeof(TftpAck),0,addrs->ai_addr,addrs->ai_addrlen)))
		{
			perror("failed to send bytes");
			exit(1);
		}
		//free ack packet
		free(ack_packet);
		//increment block count
		block_count ++ ;
		
		
		/* check if more blocks expected, else 
		 * set block_count = 0 ;
		 */

	}
	return 0 ;
}



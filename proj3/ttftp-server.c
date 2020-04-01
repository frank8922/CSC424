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

	int sockfd_l = 0,
	   sentbytes = 0;

	struct sockaddr_in my_addr, their_addr;
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct addrinfo hints, *addrs;
	int block_count ;
	
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
		if(opcode == '1')
		{
			recv_rrq_packet = (TftpReq*)buffer;
			printf("%s\n",recv_rrq_packet->filename_and_mode);
			file = openFile(recv_rrq_packet->filename_and_mode);
			if(file == NULL)
			{
				char *error_msg = malloc(MAXMSGLEN);
				sprintf(error_msg,"%s","file ");
				sprintf(error_msg,"%s",recv_rrq_packet->filename_and_mode);
				sprintf(error_msg,"%s"," not found");
				sentbytes = sendErrorPacket(FILENOTFOUND,error_msg,&their_addr,sockfd_l);
				printf("sent %d bytes\n",sentbytes);
				free(error_msg);
				break;
			}
		}
		
		/*
		* create a sock for the data packets
		*/	 
		int bytes_read = 0;
		TftpData *data_packet = NULL;
		block_count = 1 ;
		while (block_count && !feof(file)) 
		{ 
			/*
			* TODO: read from file
			*/
			data_packet = malloc(sizeof(TftpData));
			memset(data_packet->data,0,TFTP_DATALEN);
			sprintf(data_packet->opcode,"%d",TFTP_DATA);
			sprintf(data_packet->block_num,"%d",block_count);
			bytes_read = fillDataPacket(file,data_packet->data);
			/*
			* send data packet
			*/
			int sentbytes = 0;
			printf("%s\n",data_packet->data);
			sentbytes = sendDataPacket(sockfd_l,&their_addr,data_packet,bytes_read);
			printf("sent %d bytes\n",sentbytes);
			/*
			* wait for acknowledgement & DO NOT SEND PCKT IF DUP ACK
			*/
			check((recvbytes = recvfrom(sockfd_l,buffer,TFTP_DATALEN-1,0,(struct sockaddr*)&their_addr,&socksize)),"error recieving bytes");	
			printf("number of bytes recv %d\n",recvbytes);
			opcode = *((char*)buffer);
			if(opcode == '4')
			{
				TftpAck *recv_ack_packet = (TftpAck*)buffer;
				if(atoi(recv_ack_packet->block_num) == block_count)
				{
					printf("ACK: %s\n",recv_ack_packet->block_num);
				}
			}
			block_count++ ;
		}
	
	} while (!is_noloop) ;
	close(sockfd_l);
	return 0 ;
}

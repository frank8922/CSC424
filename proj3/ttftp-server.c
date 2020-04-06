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

	int sockfd_l = 0, sockfd_s = 0, sentbytes = 0, block_count = 0, y = 1;
	short opcode = 0;
	struct sockaddr_in their_addr;
	socklen_t socksize = sizeof(struct sockaddr_in);
	struct addrinfo hints, *addrs;
	char *l_port;


	/*
	 * create a socket to listen for RRQ
	 */

	//allocate space for port #
	l_port = malloc(sizeof(short));
	memset(&l_port,0,sizeof(short));

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

	check(setsockopt(sockfd_l,SOL_SOCKET,SO_REUSEADDR,&y,sizeof(y)),"failed to set socket options");

	//bind to the socket
	check((bind(sockfd_l,addrs->ai_addr,addrs->ai_addrlen)),"failed to bind socket");

	//allocate space for buffer
	void *buffer = malloc(MAXMSGLEN);
	//hold # of bytes received
	int recvbytes = 0;
	//get RRQ
	check((recvbytes = recvfrom(sockfd_l,buffer,MAXMSGLEN-1,0,(struct sockaddr*)&their_addr,&socksize)),"error recieving bytes");	

	freeaddrinfo(addrs);

	do {
	
		/*
		 * for each RRQ 
		 */

		//to hold received RRQ packet
		TftpReq *recv_rrq_packet = NULL;
		//to hold file handle
		FILE * file = NULL;
		//get first byte containting opcode
		opcode = *((uint8_t*)buffer); 

		if(!(opcode == TFTP_RRQ)) //check if opcode is a RRQ, exit if not
			break;

		//cast buffer contents to RRQ and store
		recv_rrq_packet = (TftpReq*)buffer;

		/*
		* create a sock for the data packets
		*/	 
		check((sockfd_s = socket(AF_INET,SOCK_DGRAM,0)),"failed to create send socket");
		//get address of server
		struct hostent *he = gethostbyname(inet_ntoa(their_addr.sin_addr));
		their_addr.sin_addr = *((struct in_addr*)he->h_addr);
		memset(&(their_addr.sin_zero), '\0', 8 ) ;
		
		/*
		 * parse request and open file
		 */

		//try to open file
		file = openFile(recv_rrq_packet->filename_and_mode);
		if(file == NULL)
		{
			char *error_msg = malloc(MAXMSGLEN);
			memset(error_msg,0,MAXMSGLEN);

			strcat(error_msg,"file ");
			strcat(error_msg,recv_rrq_packet->filename_and_mode);
			strcat(error_msg," not found");

			sentbytes = sendErrorPacket(FILENOTFOUND,error_msg,&their_addr,sockfd_s);
			free(error_msg);
			break;
		}

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
			opcode = htons(TFTP_DATA); //convert opcode to network format
			data_packet->opcode[0] = (opcode >> 8) & 0xff; //store second byte 
			data_packet->opcode[1] = opcode & 0xff; //store first byte
			
			//store block count
			data_packet->block_num[0] = (block_count >> 8) & 0xff; //store 1st byte
			data_packet->block_num[1] = block_count & 0xff; //store 2nd byte
			
			//read file and fill packet
			bytes_read = fillDataPacket(file,data_packet->data);
			/*
			* send data packet
			*/
			int sentbytes = 0;
			sentbytes = sendDataPacket(sockfd_s,&their_addr,data_packet,bytes_read);
			//free data packet
			free(data_packet);
			/*
			* wait for acknowledgement & DO NOT SEND PCKT IF DUP ACK
			*/
			check((recvbytes = recvfrom(sockfd_s,buffer,MAXMSGLEN-1,0,(struct sockaddr*)&their_addr,&socksize)),"error recieving bytes");	
			opcode = *((uint8_t*)buffer);
			if(opcode == TFTP_ACK) //check if opcode is an ACK
			{
        
				TftpAck *recv_ack_packet = (TftpAck*)buffer;
				short block_num = (recv_ack_packet->block_num[0] << 8) | recv_ack_packet->block_num[1]; //combine 1st & 2nd byte to form blocknum

				if(block_num == block_count)
				{
					block_count++ ;
				}
			}
		} //end while
    fclose(file); //close file	
	} while (!is_noloop) ;
	close(sockfd_l); //close listen socket
	close(sockfd_s); //close data socket
	return 0 ;
}

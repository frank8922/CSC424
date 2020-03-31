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



int send_errpack(int error_code,char *error_msg,struct sockaddr_in *client_res,int sock);
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
		TftpReq *recv_rrq_packet = (TftpReq*)buffer;
		printf("%s\n",recv_rrq_packet->filename_and_mode);

		/*if((recv_rrq_packet->filename_and_mode) == NULL)	
		{
			fprintf(stderr,"NULL POINTER (%s:%d)\n",__FILE__,__LINE__);
    		int sentbytes = send_errpack(FILENOTFOUND,"failed to find file",&their_addr,sockfd_l);
      		printf("number of sent bytes %d\n",sentbytes);
		}*/

		FILE *file;
		if((file = fopen(recv_rrq_packet->filename_and_mode,"rb")) == NULL)
		{
		perror("failed to open file");
		//send error packet back to client
		int sentbytes = send_errpack(FILENOTFOUND,"failed to find file",&their_addr,sockfd_l);
		printf("number of sent bytes %d\n",sentbytes);
		}

		

		/*
		 * create a sock for the data packets
		 */	 
		check((sockfd_s = socket(AF_INET,SOCK_DGRAM,0)),"failed to create socket");

		TftpData *data_packet = malloc(sizeof(TftpData));

		block_count = 0 ;
		while (block_count) { 

			/*
			 * TODO: read from file
			 */
			
			/*
			 * send data packet
			 */

			/*
			 * wait for acknowledgement & DO NOT SEND PCKT IF DUP ACK
			 */
			 
			block_count++ ;
		}
	
	} while (!is_noloop) ;
	return 0 ;
}

int send_errpack(int error_code,char *error_msg,struct sockaddr_in *client_res,int sock)
{
      TftpError *error_packet = malloc(sizeof(TftpError));
      sprintf(error_packet->opcode,"%d",TFTP_ERR);
      sprintf(error_packet->error_code,"%d",error_code);
      sprintf(error_packet->error_msg,"%s",error_msg);
      int sent = 0;
      int sendsock;
      check((sendsock = socket(AF_INET,SOCK_DGRAM,0)),"failed to open socket");
      check((sent = sendto(sock,error_packet,sizeof(TftpError),0,(struct sockaddr*)client_res,sizeof(struct sockaddr_in))),"failed to send error packet");
      free(error_packet);
      return sent;
}

void check(int val, char *error_msg)
{
  if(val < 0)
  {
    perror(error_msg);
  }
}

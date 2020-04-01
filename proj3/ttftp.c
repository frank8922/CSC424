/*
** name: ttftp.c
**
** author: bjr
** created: 31 jan 2015 by bjr
** last modified:
**		14 feb 2016, for 162 semester of csc424 -bjr 
**
** from template created 31 jan 2015 by bjr
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

#define USAGE_MESSAGE "usage: ttftp [-vL] [-h hostname -f filename] port"

int g_verbose = 0 ;  // global declaration; extern definition in header 

int main(int argc, char * argv[]) {
	int ch ;
	int is_server = 0 ;
	int port = 0 ; 
	int is_noloop = 0 ; 
	char * hostname = NULL ;
	char * filename = NULL ;

	// check whether we can use short as the data type for 2 byte int's
	assert(sizeof(short)==2) ;

	while ((ch = getopt(argc, argv, "vLf:h:")) != -1) {
		switch(ch) {
		case 'v':
			g_verbose ++ ;
			break ;
		case 'h':
			hostname = strdup(optarg) ;
			break ;
		case 'f':
			filename = strdup(optarg) ;
			break ;
		case 'L':
			is_noloop = 1 ;
			is_server = 1;
			break ;
		case '?':
		default:
			printf("%s\n",USAGE_MESSAGE) ;
			return 0 ;
		}
	}
	argc -= optind;
	argv += optind;

	if ( argc!= 1 ) {
			fprintf(stderr,"%s\n",USAGE_MESSAGE) ;
		exit(0) ;
	}
	port = atoi(*argv) ;

	// sanity check inputs

	/* your code */

	if (!is_server ) {
		/* is client */
		return ttftp_client( hostname, port, filename ) ;
	}
	else {
		/* is server */
		return ttftp_server( port, is_noloop ) ;
	}
	
	assert(1==0) ;
	return 0 ;
}

void validate(char* filename)
{
  if(filename == NULL || strlen(filename) > 256)
  {
      fprintf(stderr, "%s\n","Please enter a valid filename");
	  exit(1);
  }
}

int sendErrorPacket(int error_code,char *error_msg,struct sockaddr_in *client_addr,int sock)
{
      int sent = -1, len = 0;
      TftpError *error_packet = malloc(sizeof(TftpError));
      sprintf(error_packet->opcode,"%d",TFTP_ERR);
      sprintf(error_packet->error_code,"%d",error_code);
      sprintf(error_packet->error_msg,"%s",error_msg);
	  len = strlen(error_msg);
      check((sent = sendto(sock,error_packet,sizeof(error_packet)+len,0,(struct sockaddr*)client_addr,sizeof(struct sockaddr_in))),"failed to send error packet");
      free(error_packet);
      return sent;
}
int sendAckPacket(char *block_num,int sock,struct sockaddr_in *client_addr)
{
	int sentbytes = -1;
	TftpAck *ack_packet = malloc(sizeof(TftpAck));
	sprintf(ack_packet->opcode,"%d",TFTP_ACK);
	sprintf(ack_packet->block_num,"%d",block_num);
	check((sentbytes = sendto(sock,ack_packet,sizeof(TftpAck),0,&client_addr,sizeof(client_addr))),"failed to send bytes");
	free(ack_packet);
	return sentbytes;
}

int sendDataPacket(int sock,struct sockaddr_in *client_addr,TftpData *data_packet)
{
	int sentbytes = -1;
	check((sentbytes = sendto(sock,data_packet,sizeof(data_packet),0,(struct sockaddr*)&client_addr,sizeof(client_addr))),"failed to send bytes");
	printf("send bytes: %d\n",sentbytes);
	free(data_packet);
	return sentbytes;
}

TftpData* createDataPacket(FILE* file,int block_count)
{
	int bytes_read = -1;
	TftpData *data_packet = malloc(sizeof(TftpData));
	memset(data_packet->data,0,TFTP_DATALEN);
	sprintf(data_packet->opcode,"%d",TFTP_ERR);
	sprintf(data_packet->block_num,"%d",block_count);
	bytes_read += fread(data_packet->data,1,TFTP_DATALEN,file);
	printf("%d\n",bytes_read);
	return data_packet;
}

TftpReq* createRRQ(char *filename)
{
	//create new rrq packet
	TftpReq *rrq_packet = malloc(sizeof(TftpReq));
	memset(rrq_packet,0,sizeof(TftpReq));
	//set opcode
	sprintf(rrq_packet->opcode,"%d",TFTP_RRQ);
	//store filename in packet
	int len = strlen(OCTET_STRING)+1+strlen(filename)+1;
	// rrq_packet->filename_and_mode = malloc(len);
	//memset(rrq_packet->filename_and_mode,0,len);
	sprintf(rrq_packet->filename_and_mode,"%s",filename);
	//store filemode in packet
	sprintf((rrq_packet->filename_and_mode)+strlen(filename)+1,"%s",OCTET_STRING);
}

int sendRRQ(TftpReq *rrq_packet,int sock, struct sockaddr_in *client_addr)
{
	//holds # of bytes sent
	int sentbytes = 0;
	int len = strlen(rrq_packet->filename_and_mode);
	//send RRQ
	check((sentbytes=sendto(sock,rrq_packet,sizeof(rrq_packet)+len,0,(struct sockaddr*)&client_addr,sizeof(client_addr))),"failed to send bytes");
	//free packet
	// free(rrq_packet->filename_and_mode);
	free(rrq_packet);
}

FILE * openFile(char *filename,int sock,struct sockaddr_in *client_addr)
{

	FILE *file;
	if((file = fopen(filename,"rb")) == NULL)
	{
		perror("failed to open file");
		//send error packet back to client
		int sentbytes = 0;
		sentbytes = sendErrorPacket(FILENOTFOUND,"file not found",&client_addr,sock);
		printf("number of sent bytes %d\n",sentbytes);
	}
}
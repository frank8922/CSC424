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
		validate(filename);
		validatePort(port);
		return ttftp_client( hostname, port, filename ) ;
	}
	else {
		/* is server */
		validatePort(port);
		return ttftp_server( port, is_noloop ) ;
	}
	
	assert(1==0) ;
	return 0 ;
}

void check(int val, char *error_msg)
{
    if(val < 0)
    {
      perror(error_msg);
    }
}

void validate(char* filename)
{
    if(filename == NULL || strlen(filename) > 256)
    {
      fprintf(stderr, "%s\n","Please enter a valid filename");
      exit(1);
    }

}

void validatePort(int port)
{
    if(port < 1024) {
      fprintf(stderr,"%s","Port must be between: 1025 - 65535");
      exit(1);
    }
}

int sendErrorPacket(int error_code,char *error_msg,struct sockaddr_in *client_addr,int sock)
{
      int sent = -1;
      int len = strlen(error_msg)+1;
      short opcode;
      TftpError *error_packet = malloc(sizeof(TftpError));
      memset(error_packet,0,sizeof(TftpError));

      opcode = TFTP_ERR;
      opcode = htons(opcode); //convert to network format

      error_packet->opcode[0] = (opcode >> 8) & 0xff;
      error_packet->opcode[1] = opcode & 0xff;

      //store error msg
      sprintf(error_packet->error_msg,"%s",error_msg);

      //send error packet
      check((sent = sendto(sock,error_packet,(4 * sizeof(char))+len,0,(struct sockaddr*)client_addr,sizeof(struct sockaddr_in))),"failed to send error packet");

      //free error packet
      free(error_packet);

      return sent;
}
int sendAckPacket(int block_num,int sock,struct sockaddr_in *client_addr)
{
    int sentbytes = -1;
    TftpAck *ack_packet = malloc(sizeof(TftpAck));
    short opcode = htons(TFTP_ACK);

    ack_packet->opcode[0] = (opcode >> 8) & 0xff; //store second byte
    ack_packet->opcode[1] = opcode & 0xff; //store first byte

    ack_packet->block_num[0] = (block_num >> 8) & 0xff; //store second byte
    ack_packet->block_num[1] = block_num & 0xff; //store first byte

    //send ack packet
    check((sentbytes = sendto(sock,ack_packet,sizeof(TftpAck),0,(struct sockaddr*)client_addr,sizeof(struct sockaddr_in))),"failed to send bytes");

    //free packet
    free(ack_packet);
    
    return sentbytes;
}

int sendDataPacket(int sock,struct sockaddr_in *client_addr,TftpData *data_packet,int size)
{
    int sentbytes = -1;

    //send data packet
    check((sentbytes = sendto(sock,data_packet,(2 * sizeof(short)) + size,0,(struct sockaddr*)client_addr,sizeof(struct sockaddr_in))),"failed to send bytes");

    return sentbytes;
}

int fillDataPacket(FILE* file,char *data)
{
    int bytes_read = -1;
    bytes_read += fread(data,1,TFTP_DATALEN,file);
    return bytes_read;
}

TftpReq* createRRQ(char *filename)
{
    //create new rrq packet
    TftpReq *rrq_packet = malloc(sizeof(TftpReq));
    memset(rrq_packet,0,sizeof(TftpReq));

    //set opcode
    short opcode = htons(TFTP_RRQ);
    rrq_packet->opcode[0] = (opcode >> 8) & 0xff; //store 1st byte
    rrq_packet->opcode[1] = opcode & 0xff; //store 2nd byte
    
    //store filename in packet
    sprintf(rrq_packet->filename_and_mode,"%s",filename);

    //store filemode in packet
    sprintf((rrq_packet->filename_and_mode)+strlen(filename)+1,"%s",OCTET_STRING);

    return rrq_packet;
}

int sendRRQ(TftpReq *rrq_packet,int sock, struct sockaddr *client_addr)
{
    int sentbytes = -1;
    int len = sizeof(rrq_packet->filename_and_mode); 

    //send RRQ
    check((sentbytes=sendto(sock,rrq_packet,sizeof(short)+len,0,client_addr,sizeof(struct sockaddr))),"failed to send bytes");
    
    //free packet
    free(rrq_packet);

    return sentbytes;
}

FILE * openFile(char *filename)
{

    FILE *file;
    if((file = fopen(filename,"rb")) == NULL)
    {
      perror("failed to open file");
    }
    return file;
}

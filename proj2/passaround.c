/*
** name: passaround.c
**
** author: Francisco Belliard
** date: Feb 05 2020 23:25:03
** last modified:Feb 21 2020 22:29:49
**
** from template created 31 jan 2015 by bjr
**
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<assert.h>
#include<stdbool.h>

#include "passaround.h"

#define LOCALHOST "localhost" //127.0.0.1
#define MAXMSGLEN 2048 //Max message length
#define N_REPEAT_DEFAULT 1 //Repeat set to true

#define USAGE_MESSAGE "usage: passaround [-v] [-n num] [-m message] port"
#define PROG_NAME "passaround" 

typedef struct sockaddr_in inetaddr;
typedef struct sockaddr addr;
typedef struct hostent host_info;

void opensocket(int *sockfd);
void getHostname(host_info** hostinfo, char* send_addr);
char* parseHost(char** msg);
char* parsePayload(char** msg,int len_of_msg);
int sendPayload(int *sockfd, char* payload, int len_of_payload, inetaddr* sender_addr);
int recvPayload(int *sockfd,inetaddr* recv_addr,void* buffer,int buffer_len);

int g_verbose = 0 ;

int main(int argc, char * argv[]) 
{

 	int sockfd ; //socket file descriptor
	inetaddr server_addr; //holds inet protocol, address port,
	memset(&server_addr,0,sizeof(inetaddr));
	inetaddr client_addr; //holds inet protocol, address port,
									// ipv4 address, sin_zero(not used).
	host_info* clientinfo;
	int numbytes_sent; //holds the num of bytes written out
	int numbytes_recv; //holds num bytes recieved
	int ch ;
	int y = 1;
	int the_port = 0 ;
	int n_repeat = N_REPEAT_DEFAULT ; //
	char * msg = NULL ; //ponter to the msg
	char buffer[MAXMSGLEN] = { 0 }; //initialize buffer to 0;
	int is_forever = 0 ;
	
	assert(sizeof(short)==2) ; 

	/* getopt()
	 * OPTSTRING contains the option letters to be recognized; if a letter is followed by a colon, the option is expected to have an argument, which should be separated from it by white space. 
	 * When an option requires an argument, getopts places that argument into the shell variable OPTARG. */
	
	while ((ch = getopt(argc, argv, "vm:n:")) != -1)
	{
		switch(ch) {
		case 'n': // -n forward number packets to forward, then exit.
			n_repeat = atoi(optarg) ; //converts args from strings to int (optarg is )
			break ;
		case 'v': //-v verbose
			g_verbose = 1 ;
			break ;
		case 'm': //-m Take as the "message" the first received message
			msg = strdup(optarg) ; //duplcates arguments and returrns pointer to them
			break ;
		case '?':
		default:
			printf(USAGE_MESSAGE) ;
			return 0 ;
		}
	}
	argc -= optind; //decrement the total (arg count) by the opt index
	argv += optind; //increment the argv (array passed) by the opt index

	if ( argc!= 1 ) { //if incorrect command line args, throw error
		fprintf(stderr,"%s\n",USAGE_MESSAGE) ;
		exit(0) ;
	}

	the_port = atoi(*argv) ; //convert to integer and store in the_port
	printf("port: %d\n",the_port);
	assert(the_port) ;

	is_forever = (n_repeat == 0) ; //flag to terminate sending packets, 
								   //if n_repeat is 0, loop infinitely 


	int len_of_msg;
	char * send_addr;
	char * payload;

	//**parse first address from message and send payload
	if ( msg ) 
	{
		// parse and send
		len_of_msg = strlen(msg);

		 send_addr = parseHost(&msg);
		 payload = parsePayload(&msg,len_of_msg);
		//  printf("length of payload = %d\n",strlen(payload));

		 //**open socket
		 opensocket(&sockfd);
		 //**end open socket

		//**get hostname
		// getHostname(&clientinfo,send_addr);
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(the_port);
		if(inet_aton(send_addr,(struct in_addr*)&client_addr.sin_addr))
		{
			perror("failed to convert to network order");
			exit(1);
		}
		memset(&(server_addr.sin_zero),'\0',8) ;
		//**end get hostname

		
		 //**binding address to socket
		if(bind(sockfd,(addr*)&client_addr,sizeof(addr))== -1)
		{
			perror("failed to bind");
			exit(1);
		}
		 //**end binding address to socket
		
		
		 //**send payload to address
		 numbytes_sent = sendPayload(&sockfd,payload,strlen(payload),&client_addr);
		printf("S: %s:%d |%s|\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),buffer);
		 //**end send payload to address

		
		//  printf("sent %d bytes to %s\n",numbytes_sent,send_addr);

		 free(msg); 
		 free(payload);
		 n_repeat-- ; // a packet sent
	}
		// //**end parse first address from message and send payload
		// server_addr.sin_family = AF_INET ;
		// //server_addr.sin_port = htons((short)the_port);
		// server_addr.sin_addr.s_addr = INADDR_ANY ;
		// memset(&(server_addr.sin_zero),'\0',8) ;
		// if(bind(listen,(addr*)&server_addr,sizeof(addr))== -1)
		// {
		// 	perror("failed to bind");
		// 	exit(1);
		// }
	
	while( is_forever || n_repeat ) 
	{
		
		numbytes_recv = recvPayload(&sockfd,&server_addr,buffer,MAXMSGLEN);
		// printf("recv %d bytes from %s\n",numbytes_recv,inet_ntoa(server_addr.sin_addr));
		buffer[numbytes_recv]='\0';
		printf("R: %s:%d |%s|\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port),buffer);
		if(numbytes_recv > 0)	
		{
			char * buff = strdup(buffer);
			send_addr = parseHost(&buff);
			payload = parsePayload(&buff,strlen(buff));
			if(strlen(payload) > 0)
			{
				// printf("length of payload = %d\n",strlen(payload));
				// if something to send, {
				if(inet_aton(send_addr,(struct in_addr*)&server_addr.sin_addr))
				{
					perror("failed to convert to network order");
					exit(1);
				}
				numbytes_sent = sendPayload(&sockfd,payload,strlen(payload),&server_addr);
				//**end send payload to address
				// printf("sent %d bytes to %s\n",numbytes_sent,send_addr);
				printf("S: %s:%d |%s|\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port),buffer);
			}
			else
			{
				printf("no payload");
			}
			
			
			// print R: host:port |message|
		}
		//    and print S: host:port |message|
		// }

		n_repeat-- ;
	}

	close(sockfd); //close socket
	return 0 ;
}

/* param: pointer to socket file descriptor
 * throws error and exits if error.
 */
void opensocket(int *sockfd)
{
	*sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(*sockfd == -1)
	{
		perror("failed to create socket");
		exit(1) ;
	}
}

/* params: pointer to hostent struct and char array of requested hostname
 * return: true if able to get hostname, false otherwise.
 */
void getHostname(host_info** hostinfo, char* send_addr)
{
	//host_info* hp = (host_info*)malloc(sizeof(host_info*));
	if((*hostinfo = gethostbyname(send_addr))==NULL)
	{
		perror("failed to get hostname");
		exit(1);
	}
		
}

/* params: msg to parse containing host
 * return: sender address
 */
char* parseHost(char** msg)
{		
	int len_send_addr = strcspn(*msg,":"); //parse length of send address;
	char* send_addr = malloc(len_send_addr * sizeof(char));  
	if (len_send_addr < 1)
	{	
		send_addr = *msg;
	}
	else
	{
		send_addr = strtok(*msg,":"); 
	}
  
   	//parse and store send address
	//*NOTE* send_addr shares the same memory with msg, 
	//therefore free(msg) handles clean up of memory

//   printf("DEBUG:send_addr=%s n=%d\n",send_addr,len_send_addr);
	return send_addr;	
}

/* params: msg to parse, len of msg
 * return: payload;
 */
char* parsePayload(char** msg,int len_of_msg)
{
	int len_send_addr = strcspn(*msg,":"); //parse length of send address;
	int len_of_payload = len_of_msg - len_send_addr; //calculate the len of payload
	char * payload;
	bool no_payload = (len_of_payload < 1) ? false : true;
	if (!no_payload)
	{
		payload = (char *)malloc(sizeof(char)); //allocate space for payload
		payload[0] = '\0';
	}
	else
	{
		payload = (char *)malloc(len_of_payload * sizeof(char)); //allocate space for payload
		memset(payload,'\0',len_of_payload * sizeof(*payload)); //clear memory
		memcpy(payload,&((*msg)[len_send_addr+1]),sizeof(char) * len_of_payload-1); //extract and copy only payload from orignal msg
		payload[len_of_payload] = '\0'; //append null terminator
		// printf("DEBUG:payload=%s len=%d\n",payload,len_of_payload);
	}
	
	return payload;
}

/* params: socket file descriptor pointer, length of payload, sockaddr_in pointer to client
 * return: numbytes if successful, throws error and exits otherwise.
 */
int sendPayload(int* sockfd, char* payload, int len_of_payload, inetaddr* sender_addr)
{
	int numbytes_sent;
	if((numbytes_sent=sendto(*sockfd,payload,len_of_payload,0,
		(addr* )&(*sender_addr),sizeof(struct sockaddr)))== -1)
	{
		perror("failed to send packet");
		exit(1);
	}
	return numbytes_sent;
}

int recvPayload(int *sockfd,inetaddr *recv_addr,void* buffer,int buffer_len)
{
	int numbytes_recv;
	int recv_addr_len = sizeof(addr);
	
	if((numbytes_recv=recvfrom(*sockfd,buffer,MAXMSGLEN-1,0,
		(addr *)&(*recv_addr),&recv_addr_len))== -1)
	{
			perror("recvfrom");
			exit(1);
	}
	
	return numbytes_recv;
}
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
void getSenderHostname(inetaddr *,char*,int);
void setSendAddrInfo(struct addrinfo *hints,inetaddr *their_addr);
void getAddrInfo(char* send_addr,char* port,struct addrinfo *hints,struct addrinfo **servinfo);
char* parseHost(char** msg);
char* parsePayload(char** msg);
int sendPayload(int *sockfd, char* payload, int len_of_payload, struct sockaddr* sender_addr);
int recvPayload(int *sockfd,struct sockaddr * recv_addr,char** buffer,int buffer_len);

int g_verbose = 0 ;

int main(int argc, char * argv[]) 
{

 	int listen; //socket file descriptor
	inetaddr my_addr; //holds inet protocol, address port,
	inetaddr their_addr; //holds inet protocol, address port,
	host_info* he; //Hostent obj that contains port and ip_address
	struct addrinfo hints;
	struct addrinfo *servinfo;
	int numbytes_sent; //holds the num of bytes written out
	int numbytes_recv; //holds num bytes recieved
	int ch ;
	int y = 1;
	int the_port = 0 ;
	int n_repeat = N_REPEAT_DEFAULT ; //
	char * msg = NULL ; //ponter to the msg
	int is_forever = 0 ;
	char * port; 
	char * buffer = malloc(MAXMSGLEN * sizeof(char));//initialize buffer to 0;
	memset(buffer,'\0',MAXMSGLEN);
	
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
	port = (*argv); //get the port as a string
	the_port = atoi(*argv) ; //convert to integer and store in the_port
	assert(the_port) ;

	is_forever = (n_repeat == 0) ; //flag to terminate sending packets, 
								   //if n_repeat is 0, loop infinitely 

	char * send_addr;
	char * payload;

	
	opensocket(&listen);
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	memset(&(my_addr.sin_zero),'\0',8);

	if(getaddrinfo(NULL,port,&hints,&servinfo)!=0){
		perror("getaddrinfo");
		exit(1);
	}
	
	if (setsockopt(listen,SOL_SOCKET,SO_REUSEADDR,&y,sizeof(y)) == -1) { 
		perror("setsockopt");
		exit(1);
	}

	if(bind(listen,servinfo->ai_addr,servinfo->ai_addrlen) == -1 ) 
	{ perror("bind") ; exit(1) ;}


	if ( msg ) 
	{
		// parse and send

		//*parse*
		send_addr = parseHost(&msg);
		payload = parsePayload(&msg);
		//*end parse*

		if(strlen(send_addr) > 0)
		{
			
			getSenderHostname(&their_addr,send_addr,the_port);

			//**send payload to address
			numbytes_sent = sendPayload(&listen,payload,strlen(payload),(struct sockaddr* )&their_addr);
			getHostname(&he,send_addr);
			printf("S: %s:%d |%s|\n",inet_ntoa(*(struct in_addr*)he->h_addr),ntohs(their_addr.sin_port),payload);
			//**end send payload to address
		}


		 free(msg); 
		 free(send_addr);
		 if(strcmp(payload,"\0") != 0)
			free(payload);
		 freeaddrinfo(servinfo); 
		 n_repeat-- ; // a packet sent
		 
	}
		
	while( is_forever || n_repeat ) 
	{
		numbytes_recv = recvPayload(&listen,(struct sockaddr*)&my_addr,&buffer,MAXMSGLEN);

		if(numbytes_recv > 0) //if we recieved data
		{
			// print R: host:port |message|
			printf("R: %s:%d |%s|\n",inet_ntoa(my_addr.sin_addr),ntohs(my_addr.sin_port),buffer); 
			
			send_addr = parseHost(&buffer);
			payload = parsePayload(&buffer);

			if(strlen(send_addr) > 0) //if send address exists set proper settings for sending
			{
				
				setSendAddrInfo(&hints,&their_addr);
				getAddrInfo(send_addr,port,&hints,&servinfo);
				getHostname(&he,send_addr);
				//if something to send and print S: host:port |message|
				printf("S: %s:%d |%s|\n",inet_ntoa(*(struct in_addr*)he->h_addr),ntohs(their_addr.sin_port),payload); 
				numbytes_sent = sendPayload(&listen,payload,strlen(payload),servinfo->ai_addr);
				//memset(payload,'\0',strlen(payload));
				free(send_addr);
				if(strcmp(payload,"\0") != 0)
					free(payload);
				freeaddrinfo(servinfo); //free linked list created using getaddrinfo 
			}
		
			
		}
		n_repeat-- ;
	}
	free(buffer); //free the buffer memory
	close(listen); //close socket
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
 * throws error and exits if error.
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

void getSenderHostname(inetaddr *their_addr,char* send_addr,int port)
{
	//*get hostname*
	their_addr->sin_family = AF_INET;
	their_addr->sin_port = htons((short)port);
	inet_aton(send_addr,&(*their_addr).sin_addr);
	memset(&(*their_addr->sin_zero),'\0',8);
	//*end get hostname*
}

void setSendAddrInfo(struct addrinfo *hints,inetaddr *their_addr)
{

	memset(&(*hints),0,sizeof(*hints));
	hints->ai_family = AF_INET;
	hints->ai_socktype = SOCK_DGRAM;
	memset(&(*their_addr->sin_zero),'\0',8);
}

void getAddrInfo(char* send_addr,char* port,struct addrinfo *hints,struct addrinfo **servinfo)
{
	if(getaddrinfo(send_addr,port,hints,servinfo)!=0) //set the address info for the sender
	{
					perror("getaddrinfo");
					exit(1);
	}
}

/* params: msg to parse containing host
 * return: sender address
 */
char* parseHost(char** msg)
{
	char * send_addr, *temp;
	int i, len;

	if((temp = strtok(*msg,":")) != NULL)
	{
		
		for(len = 0; len < strlen(*msg); len++);
		send_addr = malloc(len+1 * sizeof(char));
		memset(send_addr,'\0',len+1);
		for(i = 0; i != len; i++)
		{ send_addr[i] = temp[i]; }
		send_addr [len+1] = '\0';

		
	}
	else
	{
		send_addr = malloc(sizeof(char));
		send_addr = "\0";
	}
	
	return send_addr;
}


/* params: msg to parse, len of msg
 * return: payload;
 */
char* parsePayload(char** msg)
{	
	char * payload; char *p;
	int i, len;
	if((p = strtok(NULL,"")) != NULL)
	{
		for(i = 0; len < strlen(p); len++);
		payload = malloc(len+1 * sizeof(char));
		memset(payload,'\0',len+1);
		for(i = 0; i != len; i++)
		{ payload[i] = p[i]; }
		payload[len+1] = '\0';
	}
	else
	{
		payload = malloc(sizeof(char));
		payload = "\0";
	}
	
	return payload;
}

/* params: socket file descriptor pointer, length of payload, sockaddr_in pointer to client
 * return: numbytes if successful, throws error and exits otherwise.
 */
int sendPayload(int* sockfd, char* payload, int len_of_payload, struct sockaddr* sender_addr)
{
	int numbytes_sent;
	int addr_len = sizeof(struct sockaddr);
	if((numbytes_sent=sendto(*sockfd,payload,len_of_payload,0,
		(addr* )&(*sender_addr),addr_len))== -1)
	{
		perror("failed to send packet");
		exit(1);
	}
	return numbytes_sent;
}

int recvPayload(int *sockfd,struct sockaddr *recv_addr,char** buffer,int buffer_len)
{
	int numbytes_recv;
	int recv_addr_len = sizeof(addr);
	
	if((numbytes_recv=recvfrom(*sockfd,*buffer,MAXMSGLEN-1,0,
		(addr *)&(*recv_addr),&recv_addr_len))== -1)
	{
			perror("recvfrom");
			exit(1);
	}
	
	return numbytes_recv;
}
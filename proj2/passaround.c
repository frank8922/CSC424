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
#define SI struct sockaddr_in //for ease of typing and to minimize long casts

#define USAGE_MESSAGE "usage: passaround [-v] [-n num] [-m message] port"
#define PROG_NAME "passaround" 

char* parseHost(char** msg);
char* parsePayload();
void check(int val, char *error_msg);
void printAddrs(struct addrinfo *res);

int g_verbose = 0 ;

int main(int argc, char * argv[]) 
{

 	
	int numbytes_sent = 0; //holds the num of bytes written out
	int numbytes_recv = 0; //holds num bytes recieved
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


	char * send_addr; //holds sender address
	char * payload; //holds payload
	int listen; //socket file descriptor
	struct sockaddr my_addr;
	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	//set server address info
	check((getaddrinfo(NULL,port,&hints,&servinfo)),"failed to resolve hostname");

	//open socket
	check((listen = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol)),"failed to open socket");

	//set socket options	
	check(setsockopt(listen,SOL_SOCKET,SO_REUSEADDR,&y,sizeof(y)),"failed to set socket options"); 

	//bind to socket to listen
	check((bind(listen,servinfo->ai_addr,servinfo->ai_addrlen)),"failed to bind to socket"); 

	if ( msg ) 
	{
		// parse and send
		send_addr = parseHost(&msg);
		payload = parsePayload();

		struct addrinfo clienthints;
		struct addrinfo *res;

		if(strlen(send_addr) > 0)
		{
			memset(&clienthints,0,sizeof(clienthints));
			clienthints.ai_family = AF_INET;
			clienthints.ai_socktype = SOCK_DGRAM;
			clienthints.ai_protocol = IPPROTO_UDP;
			check(getaddrinfo(send_addr,port,&clienthints,&res),"could not resolve host");

			//send payload to address
			check((numbytes_sent=sendto(listen,payload,strlen(payload),0,res->ai_addr,res->ai_addrlen)),"failed to send packet");
			
			//print sender address and payload
			printf("S: %s:%d |%s|\n",inet_ntoa(((SI*)res->ai_addr)->sin_addr),ntohs(((SI*)res->ai_addr)->sin_port),payload);
		}

		 //free msg
		 free(msg);
		 //free sender address
		 free(send_addr);
		 //if payload is not empty free it
		 if(strcmp(payload,"\0") != 0) 
			free(payload);
		 //free addr results
		 freeaddrinfo(res);

		 n_repeat-- ; // a packet sent
		 
	}
	
	//zero out address structure
	bzero(&my_addr,sizeof(my_addr));

	//free server info struct
	freeaddrinfo(servinfo);

	while( is_forever || n_repeat ) 
	{
		numbytes_recv = 0;
		socklen_t socksize = sizeof(struct sockaddr_in);
		
		check((numbytes_recv=recvfrom(listen,buffer,MAXMSGLEN-1,0,&my_addr,&socksize)),"error receiving bytes");

		if(numbytes_recv > 0) //if we recieved data
		{
			struct sockaddr_in * recv_addr = (SI*)&my_addr; //cast to sockaddr_in

			// print R: host:port |message|
			printf("R: %s:%d |%s|\n",inet_ntoa(recv_addr->sin_addr),ntohs(recv_addr->sin_port),buffer); 
			
			send_addr = parseHost(&buffer);
			payload = parsePayload();

			//if send address exists, set proper settings for sending
			if(strlen(send_addr) > 0) 
			{
				memset(&hints,0,sizeof(hints));
				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_DGRAM;
				hints.ai_protocol = IPPROTO_UDP;
				check(getaddrinfo(send_addr,port,&hints,&servinfo),"could not resolve host");

				//send payload to address
				check((numbytes_sent=sendto(listen,payload,strlen(payload),0,servinfo->ai_addr,servinfo->ai_addrlen)),"failed to send packet");
				
				//print S: host:port |message|
				printf("S: %s:%d |%s|\n",inet_ntoa(((SI*)servinfo->ai_addr)->sin_addr),the_port,payload); 
				free(send_addr);
				if(strcmp(payload,"\0") != 0)
					free(payload);
				freeaddrinfo(servinfo); //free linked list created using getaddrinfo 
			}
		
		}
		n_repeat-- ;
	}
	//free the buffer memory
	free(buffer); 

	//close socket
	close(listen); 
	return 0 ;
}

void printAddrs(struct addrinfo *res)
{
	void * addr;
	char ipstr[INET6_ADDRSTRLEN];
	for(struct addrinfo* p = res; p != NULL; p = p->ai_next)	
	{
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr; 
		addr = &(ipv4->sin_addr);
		printf("%s\n",inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr));
	}
}


/* 
 * params: msg to parse containing host
 * return: sender address
 */
char* parseHost(char** msg)
{
	char * send_addr, *temp;
	int i, len;

	if((temp = strtok(*msg,":")) != NULL)
	{
		for(len = 0; len < strlen(*msg); len++);
		send_addr = malloc(len+1);
		memset(send_addr,'\0',len+1);
		for(i = 0; i != len; i++)
		{ send_addr[i] = temp[i]; }
	}
	else
	{
		send_addr = malloc(1);
		send_addr = "\0";
	}
	
	return send_addr;
}


/* return: payload */
char* parsePayload()
{	
	char * payload; char *p;
	int i, len = 0;
	if((p = strtok(NULL,"")) != NULL)
	{
		for(i = 0; len < strlen(p); len++);
		payload = malloc(len+1);
		memset(payload,'\0',len+1);
		for(i = 0; i != len; i++)
		{ payload[i] = p[i]; }
	}
	else
	{
		payload = malloc(1);
		payload = "\0";
	}
	
	return payload;
}

void check(int val, char *error_msg)
{
	if (val < 0)
	{
		perror(error_msg);
		exit(1);
	}
}

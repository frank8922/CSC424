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

#include "passaround.h"

#define LOCALHOST "localhost" //127.0.0.1
#define MAXMSGLEN 2048 //Max message length
#define N_REPEAT_DEFAULT 1 //Repeat set to true
#define BUFF_SIZE 

#define USAGE_MESSAGE "usage: passaround [-v] [-n num] [-m message] port"
#define PROG_NAME "passaround" 

int g_verbose = 0 ;

int main(int argc, char * argv[]) 
{

 	int sockfd ; //socket file descriptor
	struct sockaddr_in server_addr; //holds inet protocol, address port,
	struct sockaddr_in client_addr; //holds inet protocol, address port,
					// ipv4 address, sin_zero(not used)
	struct hostent *host_ent;
	int numbytes ; //holds the number of bytes written out
	int ch ;
	int the_port = 0 ;
	int loop = 1 ; //loop on by default
	int n_repeat = N_REPEAT_DEFAULT ; //
	char * msg = NULL ; //ponter to the msg
	int is_forever = 0 ;
	
	assert(sizeof(short)==2) ; 

	/* getopt()
	 * OPTSTRING contains the option letters to be recognized; if a letter is followed by a colon, the option is expected to have an argument, which should be separated from it by white space. 
	 * When an option requires an argument, getopts places that argument into the shell variable OPTARG.
	*/
	
while ((ch = getopt(argc, argv, "vm:n:")) != -1) {
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

	//**parse first address from message and send payload
	if ( msg ) {

		// parse and send
		 int len_send_addr;
		 int len_of_payload;
		 int numbytes_sent;
		 char * send_addr;
		 char * payload;


		 len_send_addr = strcspn(msg,":"); //parse length of send address
		 len_of_payload = strlen(msg) - len_send_addr;
		 send_addr = strtok(msg,":"); //parse and store send address
		 payload = (char *)malloc(len_of_payload * sizeof(char)); //allocate space for payload
		 memset(payload,'\0',len_of_payload * sizeof(payload)); //clear memory
		 memcpy(payload,&msg[len_send_addr+1],sizeof(char) * len_of_payload-1); //extract and copy only payload from orignal msg
		 payload [len_of_payload] = '\0'; //append null terminator

		 printf("DEBUG:send_addr=%s len=%d\n",send_addr,len_send_addr);
		 printf("DEBUG:payload=%s len=%d\n",payload,len_of_payload);

		 //**get hostname
		 if ((host_ent = gethostbyname(send_addr)) == NULL)
		 {
			 perror("failed to get hostname");
			 exit(1);
		 } 
		 //**end get hostname

		//**open socket
		if ((sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1) {
			perror("failed to create socket") ;
			exit(1) ;
		}
		//**end open socket
		
		//**construct client address
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons((short)the_port);
		client_addr.sin_addr = *((struct in_addr *)host_ent->h_addr);
		memset(&(client_addr.sin_zero),'\0',8);
		//**end constructing client address


		//**bind address to socket
		if (bind(sockfd, (struct sockaddr *)&client_addr,
			sizeof(struct sockaddr)) == -1 ) 
		{
			perror("failed to bind");
			exit(1) ;
		}
		//**end binding address to socket

		if((numbytes_sent=sendto(sockfd,payload,len_of_payload,0,
			(struct sockaddr* )&client_addr,sizeof(struct sockaddr)))== -1)
		{
			perror("failed to send packet");
			exit(1);
		}

		
		printf("sent %d bytes to %s\n",numbytes_sent,send_addr);

		free(msg); 
		free(payload);
		n_repeat-- ; // a packet sent
	}
	//**end parse first address from message and send payload

		
	while( is_forever || n_repeat ) {
	
		// listen for a packet
		// print R: host:port |message|

		// if something to send, {
		//    parse and send
		//    and print S: host:port |message|
		// }

		n_repeat-- ;
	}

	close(sockfd); //close socket
	return 0 ;
}

/* 	//Specifications:

	*- The program listens for an incoming packet. 
	*- When a packet is received it decodes the list of IP address. 
	*- If the list is not empty, the program takes the first IP off the list, 
		and forward the packet to this IP, with the packet payload being the
		remaining addresses on the list. 


	//Needs:

		*- A function that initiates a connection with a server
			using everything required to read and write to sockets
		*- A function to decode packets and return ip address
		*- A function that fowards a packet to an address (payload will be reaminig addresses)
 */

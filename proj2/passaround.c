/*
** name: passaround.c
**
** author: Francisco Belliard
** date: Feb 05 2020 23:25:03
** last modified:FFeb 06 2020 01:23:49
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

#define USAGE_MESSAGE "usage: passaround [-v] [-n num] [-m message] port"

int g_verbose = 0 ;

int main(int argc, char * argv[]) {
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
	assert(the_port) ;

	is_forever = (n_repeat == 0) ; //if 
	
	if ( msg ) {

		// parse and send

		//this loop prints the args passed after specifying
		//-m flag.
		for (size_t i = 0; msg[i] != '\0'; i++)
		{
			printf("%c",msg[i]);
		}
		
		
		free(msg) ;
		n_repeat-- ; // a packet sent
	}
	
	while( is_forever || n_repeat ) {
	
		// listen for a packet
		// print R: host:port |message|

		// if something to send, {
		//    parse and send
		//    and print S: host:port |message|
		// }

		n_repeat-- ;
	}
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
/*
** netbounce-client.c
** from Beej's Guide to Network Programming: Using Unix Sockets, 
** by Brian "Beej" Hall.
**
** modified by Burt Rosenberg, 
** Created: Feb 8, 2009
** Last modified: Feb 05 2020 21:02:35 changes -bjr
**    Jan 12, 2015 - added getopt
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

#define MAXBUFLEN 100 //max buffer length

#define USAGE_MESSAGE "usage: %s [-v] -p port -h host message\n" 
#define PROG_NAME "netbounce-client" 

int main( int argc, char * argv[] ) {

	int sockfd ; //socket file descriptor
	struct sockaddr_in their_addr ; //holds inet protocol, address port,
					// ipv4 address, sin_zero(not used)
	struct hostent *he ;
	int numbytes ; //holds the number of bytes written out
	int port = 0 ; //holds the port number
	char * host = NULL ; //pointer the host
	char * msg = NULL ; //pointer to the msg
	int ch ; //used to hold chars read in
	int is_verbose = 0 ;
	
	while ((ch = getopt(argc, argv, "vp:h:")) != -1) {
		switch(ch) {
			case 'p':
				port = atoi(optarg) ;
				break ;
			case 'h':
				host = strdup(optarg) ;
				break ;
			case 'v':
				is_verbose = 1 ;
				break ;
			default:
				printf(USAGE_MESSAGE, PROG_NAME) ;
				return 0 ;
		}
	}
	argc -= optind;
	argv += optind;
	
	/* 
	 * if the host, port, or arc return false (meaning they are either NULL or don't have a value greater than 0)
	 * then print usage message.
	 */ 
	if ( !host || !port || ! argc ) {
		printf(USAGE_MESSAGE, PROG_NAME) ;
		return 0 ;
	}
	msg = strdup(*argv) ; //duplicates argv and returns a pointer to duplcate

	/* example of an assertion */
	assert(host) ; //used to signal that the queried action should evaluate to true
		       //if return false then program had a bug with that specfic part
		       //(in this case the host)
	assert(port) ;

	//attempts to get hostname, if return was null, throw error
	if ((he=gethostbyname(host))==NULL) {
		perror("gethostbyname") ;
		exit(1) ;
	}

	//check if the socket is open, if not throw error and exit
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) {
		perror("socket") ;
		exit(1) ;
	}
	their_addr.sin_family = AF_INET ; //sets the address family (ipv4 in this case).
	their_addr.sin_port = htons((short)port) ; //sets the port, the htons() swaps the numbers and puts them in
												// big endian format (network byte order).
	their_addr.sin_addr = *((struct in_addr *)he->h_addr) ; //the actual ipv4 address (32 bits).
	
	memset(&(their_addr.sin_zero), '\0', 8 ) ; /* bug with using type `socket_address_in`, must clear sin_zero otherwise lead to undefined behavior.
					 	    * sin_zero is used so struct sockaddr and sockaddr_in looks like the same from outside. 
						    * Internally sockaddr_in is much smaller than sockaddr. 
						    * Setting the rest of the bits to zero makes it look like they are the same.
						    */
	

	
	/* send message to file descriptor (write to the socket)
	 * returns number of bytes sent (num bytes written to socket)
	 * params for: sendto(socket fd, msg, msglen, int ?, their address, sizeof(sock addr)).
	 */
	if ((numbytes=sendto(sockfd, msg, strlen(msg),0,
			(struct sockaddr *)&their_addr, sizeof(struct sockaddr)) ) == -1 ) {
		perror("sendto") ;
		exit(1) ;
	}
	if (is_verbose) {
		printf("send %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr)) ;
	}
	
	{     // if you must introduce new variables, make a block
		
		struct sockaddr_in my_addr ; //holds our address
		unsigned int addr_len ; //address len
		char buf[MAXBUFLEN]; //buffer 
		
		addr_len = sizeof(struct sockaddr_in) ; 

		/*
		 * getsockname() returns the current address to which the socket sockfd is bound, in the buffer pointed to by addr. 
		 * The addrlen argument should be initialized to indicate the amount of space (in bytes) pointed to by addr.
		 * On return it contains the actual size of the socket address. 
		 * 
		 * int getsockname(int sockfd, struct sockaddr *addrsocklen_t *" addrlen );
		 */
		getsockname( sockfd, (struct sockaddr *)&my_addr, &addr_len ) ;
		if ( is_verbose ) {
			printf("sent from port %d\n", ntohs(my_addr.sin_port)) ;
			printf("calling for return packet\n") ;
		}

		addr_len = sizeof(struct sockaddr_in) ;
		if ((numbytes=recvfrom(sockfd, buf, MAXBUFLEN-1,0,
				(struct sockaddr *)&their_addr, &addr_len)) == -1 ) {
			perror("recvfrom") ;
			exit(1) ;
		}
		
		if ( is_verbose ) {
			printf("got packet from %s, port %d\n", inet_ntoa(their_addr.sin_addr), 
				ntohs(their_addr.sin_port)) ;
			printf("packet is %d bytes long\n", numbytes ) ;
		}

		buf[numbytes] = '\0' ; //terminate buffer with null terminator (so it can be printed as a string)
		printf("%s\n", buf ) ; //print whats in the buffer (i.e data that it recieved over socket)
	}
	
	close(sockfd) ; //close socket
	return 0 ;
}


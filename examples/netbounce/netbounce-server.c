/*
** netbounce-server.c
** from Beej's Guide to Network Programming: Using Unix Sockets, 
** by Brian "Beej" Hall.
**
** modified by Burt Rosenberg, 
** Created: Feb 8, 2009
** Last modified: Jan 31, 2015 - added comment about mild bug if received data is binary -bjr
**      Jan 12, 2016 - add getopt
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
#include<assert.h>

#define MAXBUFLEN 100
#define USAGE_MSG "usage: %s [-lv] -p port\n"
#define PROG_NAME "netbounce-server"  
 
int main(int argc, char * argv[]) {
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	unsigned int addr_len, numbytes;
	char buf[MAXBUFLEN];
	int ch ;
	int is_verbose = 0 ;
	int port = 0 ;
	int is_loop = 0 ;
			
	while ((ch = getopt(argc, argv, "vp:l")) != -1) {
		switch(ch) {
			case 'p':
				port = atoi(optarg) ;
				break ;
			case 'v':
				is_verbose = 1 ;
				break ;
			case 'l':
				is_loop = 1 ;
				break ;
			default:
				printf(USAGE_MSG, PROG_NAME) ;
				return 0 ;
		}
	}
	argc -= optind;
	argv += optind;
	
	if ( !port ) {
		printf(USAGE_MSG, PROG_NAME) ;
		return 0 ;
	}
	
	assert(port) ;
	
	if ((sockfd=socket(AF_INET,SOCK_DGRAM,0))==-1) {
		perror("socket") ;
		exit(1) ;
	}
	
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons((short)port) ;
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	memset(&(my_addr.sin_zero),'\0',8) ;

	if (bind(sockfd, (struct sockaddr *)&my_addr, 
		sizeof(struct sockaddr)) == -1 ) {
		perror("bind") ;
		exit(1) ;
	}

	while (1==1 ) { /* while forever */

		addr_len = sizeof(struct sockaddr) ;
		if ((numbytes=recvfrom(sockfd, buf, MAXBUFLEN-1,0,
			(struct sockaddr *)&their_addr, &addr_len)) == -1 ) {
			perror("recvfrom") ;
			exit(1) ;
		}
		// assume can be a string, and terminate
		buf[numbytes] = '\0' ;
	
		if ( is_verbose ) {
			/* added to get source port number */
			printf("got packet from %s, port %d\n", inet_ntoa(their_addr.sin_addr), 
					ntohs(their_addr.sin_port)) ;
			printf("packet is %d bytes long\n", numbytes ) ;
			
			/* Mild bug: if the incoming data was binary, the "string" might be less than numbytes long */
			printf("packet contains \"%s\"\n", buf ) ;
		}
	
		{
		   /* return the packet to sender */
			int bytes_sent ;
			if ((bytes_sent = sendto( sockfd, buf, strlen(buf), 0,
					(struct sockaddr *)&their_addr, sizeof(struct sockaddr)))==-1 ) {
				perror("send") ;
				exit(1) ;
			}
			if ( is_verbose ) {
				printf("packet sent, %d bytes\n", bytes_sent) ;
			}
		}
		
		if ( !is_loop ) break ;
	}

	close(sockfd) ;
	return 0 ;
}

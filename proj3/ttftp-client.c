/*
** name: ttftp-client.c
**
** author: 
** created:
** last modified:
**
** from template created 31 jan 2015 by bjr
**
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<assert.h>
#include<unistd.h>

#include "ttftp.h"

#define h_addr h_addr_list[0]

int ttftp_client( char * to_host, int to_port, char * file ) {
	int block_count ; 
	
	/*
	 * create a socket to send
	 */
	int fd;
	struct sockaddr_in hints
	struct sockaddr_in *addrs;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_NET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	fd = socket(AF_NET,SOCK_DGRAM,0);

	/*
	 * send RRQ
	 */

	block_count = 1 ; /* value expected */
	while ( block_count ) {
	
		/*
		 * read at DAT packet
		 */
		 
		/*
		 * write bytes to stdout
		 */
		 
		/*
		 * send an ACK
		 */

		block_count ++ ;
		
		/* check if more blocks expected, else 
		 * set block_count = 0 ;
		 */
	}
	return 0 ;
}


void check(int val,char *msg)
{
	switch (val)
	case -1 :
		perror(msg);
		exit(1);
	break;
	case 
	
}
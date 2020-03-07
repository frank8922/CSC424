/*
** name: ttftp-server.c
**
** author:
** created:
** last modified:
**
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



int  ttftp_server( int listen_port, int is_noloop ) {

	int sockfd_l;
	int sockfd_s ;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int block_count ;
	
	/*
	 * create a socket to listen for RRQ
	 */
	
	do {
	
		/*
		 * for each RRQ 
		 */
		 
		/*
		 * parse request and open file
		 */
		 
		/*
		 * create a sock for the data packets
		 */	 
		
		block_count = 1 ;
		while (block_count) { 

			/*
			 * read from file
			 */
			 
			/*
			 * send data packet
			 */
			 
			/*
			 * wait for acknowledgement
			 */
			 
			block_count++ ;
		}
	
	} while (!is_noloop) ;
	return 0 ;
}


/*
** name: ttftp.h
**
** author: bjr
** created: 31 jan 2015 by bjr
** last modified:
**   14 feb 2016, for 162 semester of csc424 -bjr 
**
*/

extern int g_verbose ;

#define MAXMSGLEN 2048
#define MAXFILENAMELEN 256

#define TFTP_RRQ 1
#define TFTP_WRQ 2
#define TFTP_DATA 3
#define TFTP_ACK 4
#define TFTP_ERR 5

#define OCTET_STRING "octet"
#define TFTP_DATALEN 512

#define ACK_TIMEOUT 4
#define ACK_RETRY 6

#define VAR_SIZE 0 
//#define IS_NULL(X) checknull((X))

typedef struct TftpReq {
	char opcode[2] ;
	char *filename_and_mode ; /* size varies */
}TftpReq;

typedef struct TftpData {
	char opcode[2] ;
	char block_num[2] ;
	char *data ; /* zero to 512 bytes */
}TftpData;

typedef struct TftpAck {
	char opcode[2] ;
	char block_num[2] ;
}TftpAck;

typedef struct TftpError {
	char opcode[2] ;
	char error_code[2] ;
	char *error_msg;
}TftpError;

int  ttftp_client( char * host, int port, char * file ) ;
int  ttftp_server( int listen_port, int is_noloop ) ;

// void check(int val,char *error_msg)
// {
// 	if(val < 0)	
// 	{
// 		perror(error_msg);
// 		exit(1);
// 	}
// }
// void checknull(void * ptr)
// {
// 	if(ptr == NULL)
// 	{
// 		fprintf(stderr,"NULL POINTER (%s:%d)\n",__FILE__,__LINE__);
// 		exit(1);
// 	}
// }
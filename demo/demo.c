#include <stdio.h>
#include <string.h>
#include "ymodem.h"

void printUsage( const char *name ){
	printf( "Usage:\n" );
	printf( "\t%s --send [tty] [filename]\n", name );
	printf( "\t%s --recv [tty] [filename]\n", name );
	printf( "\n" );
}

#define CMD_SEND  1
#define CMD_RECV  2

static void ymodemSend( const char *tty, const char *filename );
static void ymodemRecv( const char *tty, const char *filename );


int main( int argc, char *argv[] ){
	int cmd = 0;

	if( argc != 4 ){
		printUsage( argv[0] );
		return -1;
	}

	if( strcmp( argv[1], "--send" ) == 0 ){
		cmd = CMD_SEND;
	}
	else if( strcmp( argv[1], "--recv" ) == 0 ){
		cmd = CMD_RECV;
	}

	if( cmd == CMD_SEND ){
		ymodemSend( argv[2], argv[3] );
	}
	else if( cmd == CMD_RECV ){
		ymodemRecv( argv[2], argv[3] );
	}
	else{
		printUsage( argv[0] );
	}

	return 0;
}

void ymodemSend( const char *tty, const char *filename ){
	printf( "\n" );
	printf( "===============================\n" );
	printf( "YModem send:\n" );
	printf( "  file  : %s\n", filename );
	printf( "  serial: %s\n", tty );
	printf( "-------------------------------\n" );

	/* TODO */
}

void ymodemRecv( const char *tty, const char *filename ){
	printf( "\n" );
	printf( "===============================\n" );
	printf( "YModem receive:\n" );
	printf( "  file  : %s\n", filename );
	printf( "  serial: %s\n", tty );
	printf( "-------------------------------\n" );

	/* TODO */
}


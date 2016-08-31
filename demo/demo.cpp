#include <stdio.h>
#include <string.h>
#include <vector>
#include "serial/serial.h"
#include "ymodem.h"

void printUsage( const char *name ){
	printf( "Usage:\n" );
	printf( "\t%s --send [tty] [filename]\n", name );
	printf( "\t%s --recv [tty] [filename]\n", name );
	printf( "\t%s --list-ports\n", name );
	printf( "\n" );
}

#define CMD_SEND       1
#define CMD_RECV       2
#define CMD_LIST_PORTS 3

static void listPorts( void );
static void ymodemSend( const char *tty, const char *filename );
static void ymodemRecv( const char *tty, const char *filename );


int main( int argc, char *argv[] ){
	int cmd = 0;

	if( argc <= 1 ){
		printUsage( argv[0] );
		return -1;
	}

	if( strcmp( argv[1], "--send" ) == 0 ){
		if( argc != 4 ){
			printUsage( argv[0] );
			return -1;
		}
		cmd = CMD_SEND;
	}
	else if( strcmp( argv[1], "--recv" ) == 0 ){
		if( argc != 4 ){
			printUsage( argv[0] );
			return -1;
		}
		cmd = CMD_RECV;
	}
	else if( strcmp( argv[1], "--list-ports" ) == 0 ){
		if( argc != 2 ){
			printUsage( argv[0] );
			return -1;
		}
		cmd = CMD_LIST_PORTS;
	}

	if( cmd == CMD_SEND ){
		ymodemSend( argv[2], argv[3] );
	}
	else if( cmd == CMD_RECV ){
		ymodemRecv( argv[2], argv[3] );
	}
	else if( cmd == CMD_LIST_PORTS ){
		listPorts();
	}
	else{
		printUsage( argv[0] );
	}

	return 0;
}

void listPorts( void ){
	printf( "\n" );
	printf( "===============================\n" );
	printf( "List ports:\n" );
	printf( "-------------------------------\n" );

	std::vector<serial::PortInfo> ports = serial::list_ports();
	std::vector<serial::PortInfo>::iterator ite;
	for( ite=ports.begin(); ite!=ports.end(); ++ite ){
		printf( "%s : %s\n", ite->port.c_str(), ite->description.c_str() );
	}
	printf( "\n" );
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


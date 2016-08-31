#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
#include <unistd.h>
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

serial::Serial *pserial = NULL;
static int putByte( ymodem_t *ym, uint8_t bdata ){
	(void)ym;
	if( pserial == NULL ){
		printf( "WHY?\n" );
		return -1;
	}
	pserial->write( &bdata, 1 );
	return 1;
}

static int getByte( ymodem_t *ym, int timeout ){
	(void)ym;
	(void)timeout;
	if( pserial == NULL ){
		printf( "WHY?\n" );
		return -1;
	}
	uint8_t bdata;
	int cnt = pserial->read( &bdata, 1 );
	if( cnt != 1 ){
		return -1;
	}
	return bdata;
}


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

	serial::Serial serialport;
	serialport.setBaudrate( 115200 );
	serialport.setFlowcontrol( serial::flowcontrol_none );
	serialport.setBytesize( serial::eightbits );
	serialport.setStopbits( serial::stopbits_one );
	serialport.setPort( std::string(tty) );
	serial::Timeout timeout = serial::Timeout::simpleTimeout( 1000 );
	serialport.setTimeout( timeout );
	try{
		serialport.open();
	}
	catch( serial::IOException e ){
		printf( "Can't open serial port, %s.\n", e.what() );
		return;
	}
	if( !serialport.isOpen() ){
		printf( "Can't open serial port.\n" );
		return;
	}
	pserial = &serialport;
	serialport.flush();

	std::ifstream ifs( filename, std::ios::binary );
	if( !ifs.is_open() ){
		printf( "Can't open input file.\n" );
		return;
	}
	ymodem_t ym;
	ym.config.num_of_retry = 5;
	ym.config.putByte = putByte;
	ym.config.getByte = getByte;
	ym.config.timeout = 5;
	ymodem_init( &ym );
	int ret;
	ret = ymodem_startTransmit( &ym, filename, 10 );
	if( ret == YM_SUCCESS ){
		char buffer[1024];
		while( !ifs.eof() ){
			ifs.read( buffer, 1024 );
			int count = ifs.gcount();
			printf( "Read data from file %d\n", count );
			int ret = ymodem_transmit( &ym, (uint8_t*)buffer, count );
			if( ret != YM_SUCCESS ){
				break;
			}
		}
	}
	ymodem_finishTransmit( &ym );

	const char *msg = "transmit done\r\n";
	serialport.write( (uint8_t*)msg, strlen(msg) );
	serialport.close();
	pserial = NULL;
	ifs.close();
}

void ymodemRecv( const char *tty, const char *filename ){
	printf( "\n" );
	printf( "===============================\n" );
	printf( "YModem receive:\n" );
	printf( "  file  : %s\n", filename );
	printf( "  serial: %s\n", tty );
	printf( "-------------------------------\n" );

	serial::Serial serialport;
	serialport.setBaudrate( 115200 );
	serialport.setFlowcontrol( serial::flowcontrol_none );
	serialport.setBytesize( serial::eightbits );
	serialport.setStopbits( serial::stopbits_one );
	serialport.setPort( std::string(tty) );
	try{
		serialport.open();
	}
	catch( serial::IOException e ){
		printf( "Can't open serial port, %s.\n", e.what() );
		return;
	}
	if( !serialport.isOpen() ){
		printf( "Can't open serial port.\n" );
		return;
	}
}


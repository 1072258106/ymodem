#include "ymodem.h"
#include "string.h"

#define YM_ASSERT( exp )
#define YM_PDEBUG( fmt, args... )
#define YM_PERROR( fmt, args... )

/**
 * @brief  Update CRC16 for input byte
 * @param  crc_in input value 
 * @param  input byte
 * @retval None
 */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
	uint32_t crc = crc_in;
	uint32_t in = byte | 0x100;

	do
	{
		crc <<= 1;
		in <<= 1;
		if(in & 0x100)
			++crc;
		if(crc & 0x10000)
			crc ^= 0x1021;
	}

	while(!(in & 0x10000));

	return crc & 0xffffu;
}

/**
 * @brief  Cal CRC16 for YModem Packet
 * @param  data
 * @param  length
 * @retval None
 */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
	uint32_t crc = 0;
	const uint8_t* dataEnd = p_data+size;

	while(p_data < dataEnd)
		crc = UpdateCRC16(crc, *p_data++);

	crc = UpdateCRC16(crc, 0);
	crc = UpdateCRC16(crc, 0);

	return crc&0xffffu;
}

static int strLen( const char *str ){
	int len = 0;
	while( *str != 0 ){
		str ++;
		len ++;
	}
	return len;
}

static void arrayCpy( uint8_t *dest, const uint8_t *src, int len ){
	int idx;
	for( idx=0; idx<len; ++idx ){
		dest[idx] = src[idx];
	}
}

static void arraySet( uint8_t *mem, uint8_t data, int len ){
	int idx;
	for( idx=0; idx<len; ++idx ){
		mem[idx] = data;
	}
}

static int sendPacket( ymodem_t *ym, int packet_size ){
	uint16_t crc;
	int retry_cnt;

	YM_ASSERT( packet_size==YM_PACKET_SIZE_128 || packet_size==YM_PACKET_SIZE_1K );

	crc = Cal_CRC16( ym->buffer, packet_size );
	retry_cnt = 0;

	while( retry_cnt < ym->config.num_of_retry ){
		retry_cnt = 0;

		/* Send packet data */
		YM_PDEBUG( "Send packet data" );
		if( packet_size == YM_PACKET_SIZE_128 ){
			ym->config.putByte( ym, SOH );
		}
		else{
			ym->config.putByte( ym, STX );
		}

		ym->config.putByte( ym, ym->packet_idx );
		ym->config.putByte( ym, ~(ym->packet_idx) );
		for( int idx=0; idx<packet_size; ++idx ){
			ym->config.putByte( ym, ym->buffer[idx] );
		}
		ym->config.putByte( ym, crc>>8 );
		ym->config.putByte( ym, crc&0xFF );

		/* Wait ack or nack */
		YM_PDEBUG( "Wait ACK or NACK" );
		int bdata = ym->config.getByte( ym, ym->config.timeout );
		if( bdata == ACK ){
			ym->packet_idx ++;
			int cpy_size = ym->buff_idx - packet_size;
			arrayCpy( ym->buffer, ym->buffer+packet_size, cpy_size );
			ym->buff_idx = cpy_size;

			return YM_SUCCESS;
		}
		else if( bdata == NAK ){
			/* Retry */
		}
		else{
			YM_PERROR( "Expect ACK or NAK, but %x received", bdata );
			/* TODO Retry */
		}
	}

	return YM_ERROR_TIMEOUT;
}


int ymodem_init( ymodem_t *ym ){
	YM_ASSERT( ym->config.getchar != NULL );
	YM_ASSERT( ym->config.putchar != NULL );

	ym->state = YM_STATE_READY;
	ym->buff_idx = 0;
	ym->packet_idx = 0;
	do{
		int idx;
		for( idx=0; idx<YM_PACKET_SIZE_1K; ++idx ){
			ym->buffer[idx] = 0;
		}
	}while( 0 );

	/* TODO */
	return YM_SUCCESS;
}
/*
 * @brief 等待'C', 发送文件名
 * @param filenane 
 */
int ymodem_startTransmit( ymodem_t *ym, const char *filename ){
	int retry_cnt = 0;
	int bdata;
	int filename_len;
	int packet_size;
	int ret;

	YM_ASSERT( ym != NULL );

	/* 如果文件名为NULL，设置空字符串 */
	if( filename == NULL ) filename = "";
	
	filename_len = strLen( filename );
	if( filename_len > YM_PACKET_SIZE_1K ){
		return YM_ERROR_FILENAME_TOO_LONG;
	}
	packet_size = filename_len>YM_PACKET_SIZE_128 ? YM_PACKET_SIZE_1K : YM_PACKET_SIZE_128;

	/* copy filename to buffer */
	arrayCpy( ym->buffer, (const uint8_t*)filename, filename_len );
	arraySet( ym->buffer+filename_len, 0, YM_PACKET_SIZE_1K-filename_len );

	ym->buff_idx = packet_size;

	/* Send file header */
	while( retry_cnt < ym->config.num_of_retry ){
		retry_cnt ++;

		YM_PDEBUG( "Wait C" );
		bdata = ym->config.getByte( ym, ym->config.timeout );
		if( bdata < 0 ){
			/* timtout */
			continue;
		}

		if( bdata != 'C' ){
			YM_PERROR( "Expect receive 'C', but %x received", bdata );
			return -1;
		}

		ret = sendPacket( ym, packet_size );
		if( ret == YM_SUCCESS ){
		} 
	}

	/* Wait 'C' */
	retry_cnt = 0;
	while( retry_cnt < ym->config.num_of_retry ){
		retry_cnt ++;
		
		YM_PDEBUG( "Wait C" );
		bdata = ym->config.getByte( ym, ym->config.timeout );
		if( bdata == 'C' ){
			return YM_SUCCESS;
		}
		else{
			YM_PERROR( "Expect receive 'C', but %x received", bdata );
		}
	}

	return YM_ERROR_TIMEOUT;
}

int ymodem_transmit( ymodem_t *ym, const uint8_t *data, int size ){
	YM_ASSERT( ym != NULL );
	YM_ASSERT( data != NULL );
	YM_ASSERT( size > 0 );

	if( ym->state != YM_STATE_TRANSMITING ){
		return YM_ERROR_STATE;
	}

	int ret;

	while( size > 0 ){
		/* Copy data to buffer */
		int cpy_size = YM_PACKET_SIZE_1K - ym->buff_idx;
		if( cpy_size > size ){
			cpy_size = size;
		}

		arrayCpy( ym->buffer+ym->buff_idx, data, cpy_size );
		data = data + cpy_size;
		size = size - cpy_size;

		if( ym->buff_idx == YM_PACKET_SIZE_1K ){
			/* Send 1K-packet */
			YM_PDEBUG( "Send 1K-packet" );
			ret = sendPacket( ym, YM_PACKET_SIZE_1K );
			if( ret != YM_SUCCESS ){
				return ret;
			}
		}
		else if( ym->buff_idx >= YM_PACKET_SIZE_128 ){
			/* Send 128B-packet */
			YM_PDEBUG( "Send 128-packet" );
			ret = sendPacket( ym, YM_PACKET_SIZE_128 );
			if( ret != YM_SUCCESS ){
				return ret;
			}
		}
	}

	return YM_SUCCESS;
}

int ymodem_finishTransmit( ymodem_t *ym ){
	int ret;
	
	if( ym->state != YM_STATE_TRANSMITING ){
		YM_PERROR( "finishTransmit state error: %d\n", ym->state );
		return YM_ERROR_STATE;
	}

	YM_PDEBUG( "Finish transmit" );
	/* Send remain data in buffer */
	if( ym->buff_idx != 0 ){
		arraySet( ym->buffer+ym->buff_idx, 0, YM_PACKET_SIZE_1K-ym->buff_idx );

		if( ym->buff_idx > YM_PACKET_SIZE_1K ){
			ym->buff_idx = YM_PACKET_SIZE_1K;
			ret = sendPacket( ym, YM_PACKET_SIZE_1K );
		}
		else{
			ym->buff_idx = YM_PACKET_SIZE_128;
			ret = sendPacket( ym, YM_PACKET_SIZE_128 );
		}

		if( ret != YM_SUCCESS ){
			YM_PERROR( "Send error" );
		}
	}

	arraySet( ym->buffer, 0, YM_PACKET_SIZE_1K );
	
	/* Send EOT */
	for( int idx=0; idx<10; ++idx ){
		/* Send EOT */
		YM_PDEBUG( "Send EOT" );
		ym->config.putByte( ym, EOT );
		/* Wait ACK */

		YM_PDEBUG( "Wait ACK" );
		ret = ym->config.getByte( ym, ym->config.timeout );
		if( ret == ACK ){
			break;
		}
	}

	/* Send empty header */
	YM_PDEBUG( "Send empty header" );
	ymodem_startTransmit( ym, NULL );

	return YM_ERROR_TIMEOUT;
}


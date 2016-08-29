#ifndef __YMODEM_H_
#define __YMODEM_H_

#include <stdint.h>

typedef enum
{
  COM_OK       = 0x00,
  COM_ERROR    = 0x01,
  COM_ABORT    = 0x02,
  COM_TIMEOUT  = 0x03,
  COM_DATA     = 0x04,
  COM_LIMIT    = 0x05
} COM_StatusTypeDef;
/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/* Packet structure defines */
#define PACKET_HEADER_SIZE      ((uint32_t)3)
#define PACKET_DATA_INDEX       ((uint32_t)4)
#define PACKET_START_INDEX      ((uint32_t)1)
#define PACKET_NUMBER_INDEX     ((uint32_t)2)
#define PACKET_CNUMBER_INDEX    ((uint32_t)3)
#define PACKET_TRAILER_SIZE     ((uint32_t)2)
#define PACKET_OVERHEAD_SIZE    (PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE - 1)
#define PACKET_SIZE             ((uint32_t)128)
#define PACKET_1K_SIZE          ((uint32_t)1024)

/* /-------- Packet in IAP memory ------------------------------------------\
 * | 0      |  1    |  2     |  3   |  4      | ... | n+4     | n+5  | n+6  | 
 * |------------------------------------------------------------------------|
 * | unused | start | number | !num | data[0] | ... | data[n] | crc0 | crc1 |
 * \------------------------------------------------------------------------/
 * the first byte is left unused for memory alignment reasons                 */

#define FILE_NAME_LENGTH        ((uint32_t)64)
#define FILE_SIZE_LENGTH        ((uint32_t)16)

#define SOH                     ((uint8_t)0x01)  /* start of 128-byte data packet */
#define STX                     ((uint8_t)0x02)  /* start of 1024-byte data packet */
#define EOT                     ((uint8_t)0x04)  /* end of transmission */
#define ACK                     ((uint8_t)0x06)  /* acknowledge */
#define NAK                     ((uint8_t)0x15)  /* negative acknowledge */
#define CA                      ((uint32_t)0x18) /* two of these in succession aborts transfer */
#define CRC16                   ((uint8_t)0x43)  /* 'C' == 0x43, request 16-bit CRC */
#define NEGATIVE_BYTE           ((uint8_t)0xFF)

#define ABORT1                  ((uint8_t)0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  ((uint8_t)0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT             ((uint32_t)0x100000)
#define DOWNLOAD_TIMEOUT        ((uint32_t)1000) /* One second retry delay */
#define MAX_ERRORS              ((uint32_t)5)

/* Exported functions ------------------------------------------------------- */
COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size);
COM_StatusTypeDef Ymodem_Transmit(uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size);




#define YM_ASSERT( exp )
#define YM_PDEBUG( msg );

#define YM_SUCCESS                  (0)
#define YM_ERROR_TIMEOUT            (-1)
#define YM_ERROR_FILENAME_TOO_LONG  (-2)

#define YM_PACKET_SIZE_128  (128)
#define YM_PACKET_SIZE_1K   (1024)

/* State mechine define */
#define Y_STATUS_INIT           (0)
#define YM_STATUS_READY         (1)
#define YM_STATUS_TRANSMITING   (2)
#define YM_STATUS_RECEIVING     (3)

typedef struct YModem ymodem_t;

typedef struct{
	/* @brief Send a byte callback function.
	 * @param ym 
	 * @param bdata The data to be send.
	 * @ret   0: success, -1: error
	 */
	int (*putByte)( ymodem_t *ym, uint8_t bdata );
	/* @brief Receive a byte callback function.
	 * @param ym
	 * @param timeout
   * @ret   -1: error
	 */
	int (*getByte)( ymodem_t *ym, int timeout );
	int timeout;
	int num_of_retry;
}ymodem_config_t;

struct YModem{
	ymodem_config_t config;
	/* SOH/STX NUM ^NUM Data[1024] CRC CRC */
	uint8_t buffer[ 1024 ];
	int     buff_idx;
	int packet_idx;
	int status;
};

/*
 */
int ymodem_init( ymodem_t *ym );

int ymodem_startTransmit( ymodem_t *ym, const char *filename );
int ymodem_transmit( ymodem_t *ym, const uint8_t *data, int size );
int ymodem_finishTransmit( ymodem_t *ym );

int ymodem_startReceive( ymodem_t *ym, char *filename, int maxlens );
int ymodem_Receive( ymodem_t *ym, const uint8_t *buffer, int size );

#endif  /* __YMODEM_H_ */

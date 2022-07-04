#ifndef __CVORG_PROTOCOL_H__
#define __CVORG_PROTOCOL_H__
#include <stdint.h>


//////////////////////////// USER PARAMETER DEFAULTS

#ifndef SZ_RX_BUF
    #define SZ_RX_BUF       255
#endif

#ifndef WATCHDOG_LIMIT
    #define WATCHDOG_LIMIT  100
#endif

////////////////////////////

#ifndef __GNUC__
    #define __attribute__(X)
#endif

//#define type(T) __typeof__(T)

#ifdef TEST_LIBRARY
  #define packed
#else
  #define packed __attribute__((packed))
#endif


#if (SZ_RX_BUF <= 255)
    typedef uint8_t         ibuf_t;
#else
    typedef uint16_t        ibuf_t;
#endif


typedef char            buf_t;
typedef uint16_t        crc_t;

#define START_CHAR      '~'
#define ESCAPE_CHAR     '\x1b'

#define CRC_INIT        0xFFFF
#define START_CHARS_CRC 0xA900

#define NULL            ((void*)(0))

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define LITTLE_ENDIAN       // pic, x86
    #define big_endian_word(w) (__typeof__(w))( (((w) >> 8) & 0xFF) | ((w & 0xFF) << 8) )
#else
    #define BIG_ENDIAN          // arm, freescale, etc
    #define big_endian_word(w) (w)
#endif



typedef union packed {
    buf_t bytes[2];
    uint16_t word;
    } word_bytes ;


typedef enum {

        STATUS_RX_OK           = 10,        // ACK
        STATUS_RX_NOT_READY    = 11,        // if rx data while PERFORMING_CMD.                -> SYS_PERFORMING_CMD
        STATUS_RX_BAD_CHECKSUM = 12,        // if rx failed due to checksum (retransmit)       -> SYS_AWAITING_CMD
        STATUS_RX_MALFORMED    = 13,        // if rx failed due to malformed packet (code bug) -> SYS_AWAITING_CMD
        STATUS_RX_TIMEOUT      = 14,        // if rx failed due to not timeout                 -> SYS_AWAITING_CMD
        STATUS_RX_TOO_LONG     = 15,        // if packet is longer than recieve buffer         -> SYS_AWAITING_CMD
        STATUS_RX_INTERNAL_FAIL= 19,

        STATUS_CMD_VALID       = 20,        // command started
        STATUS_CMD_INVALID     = 21,
        STATUS_CMD_FAILED      = 22,        // if something bad happens while trying command   -> SYS_AWAITING_CMD

        STATUS_JOB_SUCCESS     = 30,
        STATUS_JOB_BUSY        = 31,
        STATUS_JOB_FAILED      = 32
    } StatusCode_t;


typedef enum {
        RX_AWAITING_START   = 0,            // searching for sync sequence '~~' (iRxBuf == 0)
        RX_AWAITING_RESYNC  = 1,            // searching for sync sequence, already sent error packet.
        RX_GETTING_BYTES    = 2,            // receiving data
    } RxStatus_t;


// typedef struct packed {
//         buf_t status;
//         uint16_t linenum;
//     } NackPacket_t;

// maybe change state vars to variations on signed 16-bit value: -2 is desync, -1 is idle, 0..8192 is RXing

//#define MASK_RX_SYNCED 0x02
//#define MASK_RX_NACKED 0x01

typedef union packed {
    buf_t               bytes[ SZ_RX_BUF ];
    struct packed {
        uint16_t        sz_payload;
        buf_t         payload[ SZ_RX_BUF-4 ];
    };
} RX_BUF_T ;

typedef RX_BUF_T* (*SWAP_BUF_FUNC_T)(RX_BUF_T*);
typedef void (*VOID_FUNC_T)(void);
typedef void (*TX_BYTE_FUNC_T)(buf_t);

typedef struct /* not packed */ {
    TX_BYTE_FUNC_T      p_tx_byte_func;             // pointer to uart transmit handler
    SWAP_BUF_FUNC_T     p_handle_buf_func;          // pointer to command processor function
    RX_BUF_T*           pbuf;
    ibuf_t              ibuf;
    ibuf_t              sz_payload;
    crc_t               crc;
    buf_t               PrevChar;
    uint8_t             rx_watchdog;
    uint8_t             status;
} LINK_T;

/* EXPORTED FUNCTIONS */

void rx_packet_byte( LINK_T* link, buf_t RxChar );  // call me from UART rx interrupt
void inc_rx_watchdog( LINK_T* link );               // call me from timer interrupt
void send_packet( LINK_T* link, void const* p_buf, ibuf_t sz_buf ); // call me when you want to send something
void send_response( LINK_T* link, StatusCode_t s, void const* p_buf, ibuf_t sz_buf );
void send_nack( LINK_T* link, StatusCode_t s, uint16_t linenum);

#define NACK( link, s ) send_nack( link, s, __LINE__ )

#endif

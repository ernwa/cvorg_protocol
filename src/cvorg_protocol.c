#include <stdint.h>
#include "cvorg_protocol.h"
#include <stdio.h>

const crc_t crc_table[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};


inline crc_t crc_ccitt_update( crc_t crc, buf_t b){
    return ((crc << 8) & 0xff00 ) ^ crc_table[((crc>>8)&0xff) ^ (uint8_t)b];
}


inline void send_crced_byte( LINK_T* link, crc_t* p_crc, buf_t b ){
    *p_crc = crc_ccitt_update( *p_crc, b ); //update the crc
    link->p_tx_byte_func(b);
}


inline void send_escaped_byte( LINK_T* link, buf_t c ){
    if (c == START_CHAR || c == ESCAPE_CHAR)
        link->p_tx_byte_func( ESCAPE_CHAR );
    link->p_tx_byte_func( c );
}


inline void send_escaped_crced_byte( LINK_T* link, crc_t* p_crc, buf_t c ) {
    if (c == START_CHAR || c == ESCAPE_CHAR)
        send_crced_byte( link, p_crc, ESCAPE_CHAR );
    send_crced_byte( link, p_crc, c ); //send each byte in the buffer
}


inline void send_escaped_crced_bytes( LINK_T* link, crc_t* p_crc, void const* p_buf, ibuf_t sz_buf ){
    ibuf_t i;
    for ( i = 0; i < sz_buf; i++ )   //iterate through buffer
        send_escaped_crced_byte( link, p_crc, ( (buf_t const *)p_buf )[i] ); //send each byte in the buffer
}

inline void send_crc( LINK_T* link, uint16_t crc ){
    crc = big_endian_word(crc);
    send_escaped_byte( link, ((buf_t*)&crc)[0] );                               // high byte
    send_escaped_byte( link, ((buf_t*)&crc)[1] );                               // low byte
}

inline crc_t send_packet_header( LINK_T* link, ibuf_t sz_buf ){
    uint16_t crc = START_CHARS_CRC;
    link->p_tx_byte_func( START_CHAR );                                         // sync pattern
    link->p_tx_byte_func( START_CHAR );

    if (sz_buf >= 224)
        send_escaped_crced_byte( link, &crc, (buf_t)((sz_buf >> 8) | 0xE0) );   // payload size
    send_escaped_crced_byte( link, &crc, (buf_t)sz_buf );
    return crc;
}

void send_packet( LINK_T* link, void const* p_buf, ibuf_t sz_buf ){
    crc_t crc = send_packet_header( link, sz_buf );
    send_escaped_crced_bytes( link, &crc, p_buf, sz_buf  );
    send_crc( link, crc );                // payload data
}

void send_response( LINK_T* link, StatusCode_t s, void const* p_buf, ibuf_t sz_buf ){
    crc_t crc = send_packet_header( link, sz_buf + 1 );
    send_escaped_crced_byte( link, &crc, (buf_t)s );
    if (p_buf)
        send_escaped_crced_bytes( link, &crc, p_buf, (ibuf_t)sz_buf );     // payload data
    send_crc( link, crc );
}

void send_nack(LINK_T* link, StatusCode_t s, uint16_t linenum){
    if (link->status != RX_AWAITING_RESYNC){  // already sent one at least
            if (linenum)
                send_response( link, s, &linenum, sizeof(uint16_t) );
            else
                send_response( link, s, NULL, 0 );
    }
}

void inc_rx_watchdog( LINK_T* link ){      // call from timer interrupt
    if (  link &&
          link->status == RX_GETTING_BYTES &&
          ++link->rx_watchdog >= WATCHDOG_LIMIT){

        link->status = RX_AWAITING_START;
        link->sz_payload = 0;
        link->ibuf = 0;
        link->rx_watchdog = 0;
        NACK(link, STATUS_RX_TIMEOUT);
    }
}


void rx_packet_byte( LINK_T* link, buf_t RxChar ){                      // call from serial receive interrupt
    buf_t PrevChar = link->PrevChar;

    if ( !link ){
        NACK( link, STATUS_RX_INTERNAL_FAIL );
        return;
    }
    link->PrevChar = RxChar;
    link->rx_watchdog = 0;

    if ( PrevChar == START_CHAR && RxChar == START_CHAR ){  // detect sync sequence ~~
        if (link->status == RX_GETTING_BYTES){  // unexpected resync, throw error
//                printf("unexpected start sequence\n");
            NACK( link, STATUS_RX_MALFORMED );
            // don't change state because you just got a sync
        }
        link->status = RX_GETTING_BYTES;
        link->crc = START_CHARS_CRC; // that's the CRC of just '~~'
//            printf("got valid start sequence\n");
        link->PrevChar = '\0';
        goto reset_recvr;
    }

    switch( link->status ){
        case RX_GETTING_BYTES:
            break;

        case RX_AWAITING_START:
            if (RxChar != START_CHAR ){
                NACK( link, STATUS_RX_MALFORMED );
                goto desynchronize;
                }
                // fall through
        case RX_AWAITING_RESYNC:
            goto next_char;

        default:
            NACK( link, STATUS_RX_INTERNAL_FAIL );
            goto desynchronize;
    }

    // at this point we should be only getting valid characters within a frame
    if ( link->ibuf < link->sz_payload + 2 ){               // payload not yet complete
//        uint16_t old_crc = link->crc;
        link->crc = crc_ccitt_update( link->crc, RxChar );
    //    printf("%4x (+) %2x = %4x\n", old_crc, RxChar, link->crc);
    }

    // handle escaping of start and escape characters

    if( PrevChar == ESCAPE_CHAR )
        link->PrevChar = '\0';   // it doesn't matter what this character is, it goes through
    else {
        if (RxChar == ESCAPE_CHAR)     // escape next character
            goto next_char;
        if (RxChar == START_CHAR){
        //        printf("unescaped start char in packet\n");
            NACK( link, STATUS_RX_MALFORMED );
            goto desynchronize;
        }
    }

    // now we are actually getting characters, we need somewhare to store them.

    if (!link->pbuf){                                       // no buffer to put things into
        NACK( link, STATUS_RX_NOT_READY );
        goto desynchronize;
    }

    // handle the length field (1 or 2 bytes)

    if (link->ibuf == 0){                                    // got byte 0
        if ((uint8_t)RxChar < 0xE0){                        // handle 1-byte size (0-223)
            link->pbuf->bytes[0] = 0;
            link->pbuf->bytes[1] = RxChar;
            link->ibuf = 1;
        }
        else {
            link->pbuf->bytes[0] = RxChar & 0x1F;           // handle 2-byte size (0-8191)
        }
    }
    else {
        link->pbuf->bytes[ link->ibuf ] = RxChar;           // got byte 1-?
    }


    if (++link->ibuf == 2){                                 // got the length
       link->pbuf->sz_payload = big_endian_word(link->pbuf->sz_payload);
       link->sz_payload = (ibuf_t)link->pbuf->sz_payload;
       if ( link->sz_payload >= (SZ_RX_BUF - 4) ){
           NACK( link, STATUS_RX_TOO_LONG );
           goto desynchronize;
       }
    }


    if (link->ibuf == (link->sz_payload + 4)){              // got CRC, now in possession of full packet
        buf_t* crc_ptr = &link->pbuf->bytes[ link->sz_payload + 2 ];
        word_bytes packet_crc = { { crc_ptr[0], crc_ptr[1] } };
//        printf("packet_crc=%4x, link->crc=%4x, reordered=%4x\n", packet_crc.word, link->crc, big_endian_word(link->crc));
        if ( big_endian_word(link->crc) == packet_crc.word ){
            RX_BUF_T* pbuf = link->pbuf;
            link->pbuf = 0;                   // in case interrupt becomes reentrant
            link->pbuf = link->p_handle_buf_func(pbuf); // TODO: try returning pointer so func can be called from recieve interrupt?
        }
        else {
            NACK( link, STATUS_RX_BAD_CHECKSUM );
            goto desynchronize; // can't guarantee we're synchronized
        }

        link->status = RX_AWAITING_START;
        link->PrevChar = '\0';
        goto reset_recvr;

    }
    goto next_char;

desynchronize:
    link->status = RX_AWAITING_RESYNC;
reset_recvr:
    link->ibuf = 0;
    link->sz_payload = 0;
next_char:
    ;
}

#include "threaded_framing.h"

// uart 1 <--> uart 2 example

// both define a send_char function and a rx_char interrupt
// a timer generates 1 - 10 ms interrupts on timer1_int()

#define INTERRUPT
uint8_t volatile * const uart1_rx_reg = (void*)0x1000;
uint8_t volatile * const uart2_rx_reg = (void*)0x1010;
uint8_t volatile * const uart1_tx_reg = (void*)0x1001;
uint8_t volatile * const uart2_tx_reg = (void*)0x1011;


// definitions

RX_BUF_T* uart1_handle_frame( RX_BUF_T* pbuf );
void uart1_tx_char( buf_t c);
LINK_T uart1_link = { uart1_tx_char, uart1_handle_frame }; // rest will be zero


RX_BUF_T* uart2_handle_frame( RX_BUF_T* pbuf );
void uart2_tx_char( buf_t c);
LINK_T uart2_link = { uart2_tx_char, uart2_handle_frame };

// hardware interface functions

void uart1_tx_char( buf_t c){
  *uart1_tx_reg = c;
}

void INTERRUPT uart1_rx_char(void){
  rx_packet_byte( &uart1_link, *uart1_rx_reg );
}


void uart2_tx_char( buf_t c){
  *uart2_tx_reg = c;
}

void INTERRUPT uart2_rx_char(void){
  rx_packet_byte( &uart2_link, *uart2_rx_reg );
}

void INTERRUPT timer1_int(void){
  inc_rx_watchdog( &uart1_link );
  inc_rx_watchdog( &uart2_link );
}

/// -------- actual command processing


RX_BUF_T* uart1_handle_frame( RX_BUF_T* pbuf ){
  send_packet( &uart2_link, &pbuf->payload, pbuf->sz_payload );
  return pbuf;
}

RX_BUF_T* uart2_handle_frame( RX_BUF_T* pbuf ){
  send_packet( &uart1_link, &pbuf->payload, pbuf->sz_payload );
  return pbuf;
}

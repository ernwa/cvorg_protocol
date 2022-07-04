
u8 rx_handle_interrupt( LINK_T* link, u32 UartBaseAddress )
{
    StatusRegister = XUartLite_GetStatusReg( UartBaseAddress);

    while( !XUartLite_IsRecieveEmpty( UartBaseAddress ) )
    {
        func = rx_packet_byte( link, (buf_t)XUartLite_ReadReg( UartBaseAddress, XUL_RX_FIFO_OFFSET ) )
        if (func)
        {
            // enable interrupts
        }
    }
}
// #pragma (__packed__)
// struct _STAT_REG_FLAGS {
//     bool rx_data_valid:1;
//     bool rx_fifo_full:1;
//     bool tx_fifo_empty:1;
//     bool tx_fifo_full:1;
//     bool interrupt_enabled:1;
//     bool overrun_error:1;
//     bool frame_error:1;
//     bool parity_error:1;
//     u_int32_t reserved:24;
// };
//
// union STAT_REG {
//     _STAT_REG_FLAGS flags;
//     u_int32_t word;
// };
//
// struct UART_REGISTERS {
//     u32 volatile const rx_fifo;
//     u32 volatile tx_fifo;
//     u32 volatile const stat_reg;
//     u32 volatile ctrl_reg;
// }

//Interrupt for the first UART controller
void uart0_handler(void *CallbackRef)   // interrupts are disabled
{
    (void)CallbackRef; // Unused (avoid compilation error)
    u32 volatile R14_register = mfgpr(r14); // why is this declared volatile?
//    XIntc_Acknowledge(&Intc, UARTLITE_INT_ID_0);
    if( !XUartLite_IsRecieveEmpty(BaseAddress) )    // got a character
    {
        buf_t rx_char = XUartLite_ReadReg(BaseAddress, XUL_RX_FIFO_OFFSET);
        rx_packet_byte( &link_pc, rx_char );
    }
}


RX_BUF_T* handle_uart0_command( RX_BUF_T* pbuf )
{
    // handle buffer


    return pbuf;
}

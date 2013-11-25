/***************************************************
* FileName:        can_to_rs232_converter.h
* Dependencies:    See INCLUDES section below
* Processor:       PIC18
* Compiler:        C18 3.43
* Company:         SpinItalia s.r.l.
* 
* All Rights Reserved.
*
*  The information contained herein is confidential 
* property of SpinItalia s.r.l. The user, copying, transfer or 
* disclosure of such information is prohibited except
* by express written agreement with SpinItalia s.r.l.
*
* First written on 18/10/2012 by Luca Lucci.
*
* Module Description:
* Define prototype and const for can_to_rs232_converter.h
*
****************************************************/

#ifndef _CAN_TO_RS232_CONVERTER
#define _CAN_TO_RS232_CONVERTER

/** I N C L U D E S ********************************/
#include "uart_interface.h"
#include "can_interface.h"
#include "../system/io_cfg.h"

/** V A R I A B L E S  *****************************/
extern unsigned char uart_number;	// number of serial driver used
extern unsigned char uart_interrupt_tx;
extern unsigned char uart_interrupt_rx;
extern unsigned long uart_baud_rate;
extern struct uart_status * uartx_status;	// pointer to the uart_status structur

/** D E F I N E ************************************/
// Choose which uart use
#define UART1 1
#define UART2 2

#define UART_TO_CONVERTER UART1

// Header's value to send message to/from pc interface
#define HEADER	'$'

/** M A C R O **************************************/
// I don't want to worry about number of serial port used, so I define generic 
// function
#define uart_init()	{if(uart_number == 1) uart1_init(); else uart2_init();}
#define uart_open(baud_rate) {if(uart_number == 1) uart1_open(baud_rate); else uart2_open(baud_rate);}
#define uart_close() {if(uart_number == 1) uart1_close(); else uart2_close();}
#define uart_buffer_tx_load(return_value, byte) {if(uart_number == 1) return_value = uart1_buffer_tx_load(byte); else return_value = uart2_buffer_tx_load(byte);}
#define uart_buffer_tx_seq_load(return_value, data_write, length) {if(uart_number == 1) return_value = uart1_buffer_tx_seq_load(data_write, length); else return_value = uart2_buffer_tx_seq_load(data_write, length);}
#define uart_get_tx_buffer_empty_space(return_value) {if(uart_number == 1) return_value = uart1_get_tx_buffer_empty_space(); else return_value = uart2_get_tx_buffer_empty_space();}
#define uart_buffer_rx_load() {if(uart_number == 1) uart1_buffer_rx_load(); else uart2_buffer_rx_load();}
#define uart_buffer_read(byte) {if(uart_number == 1) uart1_buffer_read(byte); else uart2_buffer_read(byte);}
#define uart_isr() {if(uart_number == 1) uart1_isr(); else uart2_isr();}

/** S T R U C T  ***********************************/

/** P R O T O T Y P E S ****************************/
void initialize_user(void);		// user's application initializations
void uart_to_canbus_process(void);	// process uart and canbus  message

#endif


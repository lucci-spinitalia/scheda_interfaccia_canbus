/***************************************************
* FileName:        uart_interface.h
* Dependencies:    See INCLUDES section below
* Processor:       PIC18
* Compiler:        C18 3.43
* Company:         SpinItalia s.r.l.
* 
* All Rights Reserved.
*
* The information contained herein is confidential 
* property of SpinItalia s.r.l. The user, copying, transfer or 
* disclosure of such information is prohibited except
* by express written agreement with SpinItalia s.r.l.
*
* First written on 18/10/2012 by Luca Lucci.
*
* Module Description:
* Define prototype and const for uart_interface.c
*
****************************************************/

#ifndef _UART_INTERFACE_H
#define _UART_INTERFACE_H

/** I N C L U D E S ********************************/
#include <usart.h>

/** D E F I N E ************************************/
#define UART_BUFFER_SIZE_TX 10
#define UART_BUFFER_SIZE_RX 10

#define UART1_BAUD_CONFIG BAUD_16_BIT_RATE & BAUD_AUTO_OFF & BAUD_WAKEUP_OFF & BAUD_IDLE_RX_PIN_STATE_HIGH & BAUD_IDLE_TX_PIN_STATE_HIGH
#define UART2_BAUD_CONFIG BAUD_16_BIT_RATE & BAUD_AUTO_OFF & BAUD_WAKEUP_OFF & BAUD_IDLE_RX_PIN_STATE_HIGH & BAUD_IDLE_TX_PIN_STATE_HIGH
#define UART1_CONFIG USART_TX_INT_OFF & USART_RX_INT_OFF & USART_BRGH_HIGH & USART_CONT_RX & USART_EIGHT_BIT & USART_ASYNCH_MODE
#define UART2_CONFIG USART_TX_INT_OFF & USART_RX_INT_OFF & USART_BRGH_HIGH & USART_CONT_RX & USART_EIGHT_BIT & USART_ASYNCH_MODE

/* Baud rate BR with Sync = 0
*  BRG16    BRGH    MODE          BR FORMULA
*    0       0      8 bit/Async    Fosc/[64([SPBRGHx:SPBRGx]+1)]
*    0       1      8 bit/Async    Fosc/[16([SPBRGHx:SPBRGx]+1)]
*    1       0      16 bit/Async   Fosc/[16([SPBRGHx:SPBRGx]+1)]
*    1       1      16 bit/Async   Fosc/[4([SPBRGHx:SPBRGx]+1)]
*
* In Fosc must be taken in account the pll, too. With a 16MHz oscillator and a 
* 4x pll enabled, the reference Fosc in table 22-4 is 64 MHz
*	6666	// 2400	
*	1666	// 9600
*	832		// 19200
*	278		// 57600
*	138		// 115200
*/

#define UART1_BAUD_RATE 138	// 115200
#define UART2_BAUD_RATE 138	// 115200

#define UART1_INTERRUPT_TX (UART1_CONFIG&0x80)
#define UART1_INTERRUPT_RX (UART1_CONFIG&0x40)
#define UART2_INTERRUPT_TX (UART2_CONFIG&0x80)
#define UART2_INTERRUPT_RX (UART2_CONFIG&0x40)

/** M A C R O     **********************************/
#define uart1_open()	Open1USART( UART1_CONFIG, UART1_BAUD_RATE )
#define uart2_open()	Open2USART( UART2_CONFIG, UART2_BAUD_RATE )

/** S T R U C T S **********************************/
extern struct status
{
  unsigned buffer_tx_full 		:1;	// tx buffer full
  unsigned buffer_tx_empty 		:1; // tx buffer empty
  unsigned buffer_rx_full		:1; // rx buffer full
  unsigned buffer_rx_empty		:1; // rx buffer empty
  unsigned buffer_rx_overflow	:1; // rx buffer full and uart still receive data
  unsigned buffer_rx_error		:1; // OERR or FERR error accured. User needs to
										// clear this error-bit (uart_rx_error) in 
										// the fw
};

/** S H A R E D  V A R I A B L E S  ****************/
extern struct status uart1_status;	// uart's status flag
extern struct status uart2_status;	// uart's status flag

extern unsigned char uart1_buffer_tx[UART_BUFFER_SIZE_TX];	//tx buffer
//extern unsigned char uart1_buffer_tx_data_cnt;	// number of byte to transmit
// write position in tx buffer. Differ from uart1_buffer_tx_data_cnt because 
// it's circular and it can be restart from 0
//extern unsigned char uart1_buffer_tx_wr_ptr;	
//extern unsigned char uart1_buffer_tx_rd_ptr;	// read position to place data from
											    // buffer to TXREG
extern unsigned char uart1_buffer_rx[UART_BUFFER_SIZE_RX];	//rx buffer
//extern unsigned char uart1_buffer_rx_data_cnt;	// number of byte received
//extern unsigned char uart1_buffer_rx_wr_ptr;	// write position in rx buffer
//extern unsigned char uart1_buffer_rx_rd_ptr;	// read position by the application

extern unsigned char uart2_buffer_tx[UART_BUFFER_SIZE_TX];	//tx buffer
//extern unsigned char uart2_buffer_tx_data_cnt;	// number of byte to transmit
// write position in tx buffer. Differ from uart2_buffer_tx_data_cnt because 
// it's circular and it can be restart from 0
//extern unsigned char uart2_buffer_tx_wr_ptr;	// write position in tx buffer
//extern unsigned char uart2_buffer_tx_rd_ptr;	// read position to place data from
											// buffer to TXREG
extern unsigned char uart2_buffer_rx[UART_BUFFER_SIZE_RX];	//rx buffer
//extern unsigned char uart2_buffer_rx_data_cnt;	// number of byte received
//extern unsigned char uart2_buffer_rx_wr_ptr;	// write position in rx buffer
//extern unsigned char uart2_buffer_rx_rd_ptr;	// read position by the application

/** P R O T O T Y P E S ****************************/
unsigned char uart1_buffer_read(unsigned char *);	// returns a char from rx buffer
unsigned char uart1_buffer_rx_load(void);	// read a char from usart
unsigned char uart1_get_rx_data_size(void);		// returns the number of 
											    // characters in receive buffer
void uart1_buffer_send(void);			// send a byte from tx buff to usart
unsigned char uart1_buffer_tx_load(unsigned char *);		// put a char in tx buf
unsigned char uart1_get_tx_buffer_empty_space(void);	// returns size of empty 
														// section in tx buffer
void uart1_isr(void);					// called from ISR from the main program


unsigned char uart2_buffer_read(unsigned char *);	// returns a char from rx buffer
unsigned char uart2_buffer_rx_load(void);		// read a char from usart
unsigned char uart2_get_rx_data_size(void);		// returns the number of 
											    // characters in receive buffer
void uart2_buffer_send(void);	// send a byte from tx buff to usart
unsigned char uart2_buffer_tx_load(unsigned char *);		// put a char in tx buf
unsigned char uart2_get_tx_buffer_empty_space(void);	// returns size of empty 
														// section in tx buffer

void uart2_isr(void);					// called from ISR from the main program

void uart_init(void);					// initialization of uart module

#endif


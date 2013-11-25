/***************************************************
* FileName:        can_interface.h
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
* Define prototype and const for can_interface.c
*
****************************************************/

#ifndef _CAN_INTERFACE_H
#define _CAN_INTERFACE_H

/** I N C L U D E S ********************************/

/** M A C R O     **********************************/
#define can_bus_enable_terminator	can_bus_res = 1
#define can_bus_disable_terminator	can_bus_res = 0

/** D E F I N E ************************************/
#define CAN_BUFFER_SIZE_TX 2
#define CAN_BUFFER_SIZE_RX 128

// Configuration bit masks to be 'ored' together and passed as the parameter to
// the 'can_init_filter_enable' routine. 
#define RXF_ALL_ON	0b1111111111111111
#define RXF0_ON		0b0000000000000001
#define RXF1_ON		0b0000000000000010
#define RXF2_ON		0b0000000000000100
#define RXF3_ON		0b0000000000001000
#define RXF4_ON		0b0000000000010000
#define RXF5_ON		0b0000000000100000
#define RXF6_ON		0b0000000001000000
#define RXF7_ON		0b0000000010000000
#define RXF8_ON		0b0000000100000000
#define RXF9_ON		0b0000001000000000
#define RXF10_ON	0b0000010000000000
#define RXF11_ON	0b0000100000000000
#define RXF12_ON	0b0001000000000000
#define RXF13_ON	0b0010000000000000
#define RXF14_ON	0b0100000000000000
#define RXF15_ON	0b1000000000000000
#define RXF_ALL_OFF 0b0000000000000000

// Configuration bit masks to be 'ored' together and passed as the parameter to
// the 'can_init_prog_buffer_mode' routine
#define CAN_PROG_B0_RX					0b00000000
#define CAN_PROG_B1_RX					0b00000000
#define CAN_PROG_B2_RX					0b00000000
#define CAN_PROG_B3_RX					0b00000000
#define CAN_PROG_B4_RX					0b00000000
#define CAN_PROG_B5_RX					0b00000000
#define CAN_PROG_Bn_RX					0b00000000
#define CAN_PROG_B0_TX					0b00000100
#define CAN_PROG_B1_TX					0b00001000
#define CAN_PROG_B2_TX					0b00010000
#define CAN_PROG_B3_TX					0b00100000
#define CAN_PROG_B4_TX					0b01000000
#define CAN_PROG_B5_TX					0b10000000
#define CAN_PROG_Bn_TX					0b11111100

// Configuration bit masks to be 'ored' together and passed as the parameter to
// the 'can_init_interrupt' routine 
#define CAN_INTERRUPT_PROG_ALL_ON		0b11111100
#define CAN_INTERRUPT_PROG0_ON			0b00000100
#define CAN_INTERRUPT_PROG1_ON			0b00001000
#define CAN_INTERRUPT_PROG2_ON			0b00010000
#define CAN_INTERRUPT_PROG3_ON			0b00100000
#define CAN_INTERRUPT_PROG4_ON			0b01000000
#define CAN_INTERRUPT_PROG5_ON			0b10000000
#define CAN_INTERRUPT_PROG_ALL_OFF		0b00000000

#define CAN_INTERRUPT_RX_ALL_ON			0b00000011
#define CAN_INTERRUPT_RX1_ON			0b00000010
#define CAN_INTERRUPT_RX0_ON			0b00000001
#define CAN_INTERRUPT_RX_ALL_OFF		0b00000000

// Configuration bit masks to be 'ored' together and passed as the parameter to
// the 'can_init_interrupt' routine. 
#define CAN_INTERRUPT_TX_ALL_ON			0b00011100
#define CAN_INTERRUPT_TX0_ON			0b00000100
#define CAN_INTERRUPT_TX1_ON			0b00001000
#define CAN_INTERRUPT_TX2_ON			0b00010000
#define CAN_INTERRUPT_TX_ALL_OFF		0b00000000

// Configuration bit masks to be 'ored' together and passed as the parameter to
// the 'can_init_interrupt' routine. 
#define CAN_INTERRUPT_ERROR_ON				0b00100000
#define CAN_INTERRUPT_ERROR_OFF				0b00000000
#define CAN_INTERRUPT_WAKE_UP_ACTIVITY_ON	0b01000000
#define CAN_INTERRUPT_WAKE_UP_ACTIVITY_OFF	0b00000000
#define CAN_INTERRUPT_MESSAGE_ERROR_ON	 	0b10000000  // use this only for baud rate determination
														// and in conjunction with Listen Only Mode
#define CAN_INTERRUPT_MESSAGE_ERROR_OFF		0b00000000

// define which interrupts enable. To enable interrup it must be defined CAN_INTERRUPT_TX
// and CAN_INTERRUPT_RX
#define CAN_CONFIG CAN_INTERRUPT_TX_ALL_OFF | CAN_INTERRUPT_RX_ALL_OFF | CAN_INTERRUPT_ERROR_ON | CAN_INTERRUPT_MESSAGE_ERROR_OFF
#define CAN_CONFIG_PROG CAN_INTERRUPT_PROG_ALL_OFF | CAN_INTERRUPT_RX_ALL_OFF

// Intrrupt enable flags
//#define CAN_INTERRUPT_TX
//#define CAN_INTERRUPT_RX

/** S T R U C T  ***********************************/
extern struct CAN_STATUS
{
  unsigned buffer_tx_full 				:1;	// tx buffer full
  unsigned buffer_tx_empty 				:1; // tx buffer empty
  unsigned buffer_tx_error_warning		:1; // tx error > 95
  unsigned buffer_tx_error_passive		:1; // tx error > 127
  unsigned buffer_tx_error_off			:1; // tx error > 255

  unsigned buffer_rx_full				:1; // rx buffer full
  unsigned buffer_rx_empty				:1; // rx buffer empty
  unsigned buffer_rx_error_generic		:1; // rx message error
  unsigned buffer_rx_error_overflow		:1; // rx buffer full and can still receive data
  unsigned buffer_rx_error_warning		:1; // rx error > 35
  unsigned buffer_rx_error_passive		:1; // rx error > 127
};

extern struct CAN_MESSAGE_STRUCT
{
  //if exide flag is clear, it must read only first 11 bits of eid to retreive 
  //sid
  unsigned long id;
  unsigned char data[8];
  unsigned char data_length;

  unsigned char filter_hit;

  unsigned char exid_frame;
  unsigned char rtr_frame;
};

/** S H A R E D  V A R I A B L E  ******************/
typedef struct CAN_MESSAGE_STRUCT CAN_MESSAGE;

extern struct CAN_STATUS can_status;	// can's status flag

extern CAN_MESSAGE can_buffer_tx[CAN_BUFFER_SIZE_TX];	//tx buffer
extern CAN_MESSAGE can_buffer_rx[CAN_BUFFER_SIZE_RX];	//rx buffer

/** P R O T O T Y P E S ****************************/
// initialization of uart module
unsigned char can_init(unsigned char global_interrupt, unsigned char prog_interrupt,
					   unsigned char prog_buffer_mode, unsigned char* sync_param);

// puts data in tx buffer
unsigned char can_buffer_tx_load(CAN_MESSAGE *data_write);

// send the message store in tx buffer over can bus
unsigned char can_buffer_send(void);

// returns size of empty section in tx buffer
unsigned char can_get_tx_buffer_empty_space(void);

// read a message from can
unsigned char can_buffer_rx_load(void);

// returns the message from rx buffer	
unsigned char can_buffer_read(CAN_MESSAGE *data_read);

// return error messages
unsigned char can_error_handle(unsigned char *error_message, unsigned char *length);

// called from ISR from the main program
void can_isr(void);

// Initialize mask and filter				
extern void can_init_mask_id(unsigned char mask, unsigned long mask_id, 
					  		 unsigned char exiden);

// enable selected filter
extern void can_init_filter_enable(unsigned int filter);

//associate filter to an rx buffer
extern unsigned char can_init_filter_buffer(unsigned char filter, unsigned char buffer);
// associate filter to mask
extern void can_init_filter_mask(unsigned char filter, unsigned char mask);
// initialize filter ID
extern void can_init_filter_id(unsigned char filter, unsigned long filter_id, 
							   unsigned char exiden);

// initialize baudrate and bit timing
extern unsigned char can_init_baudrate_set(unsigned char SJW, unsigned char PRSEG,
									unsigned char PS1, unsigned char PS2, 
									unsigned char brp);


#endif
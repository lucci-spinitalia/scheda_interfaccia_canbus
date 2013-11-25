/***************************************************
* FileName:        can.h
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
#include "can.h"

/** M A C R O     **********************************/
#define can_bus_enable_terminator	can_bus_res = 1
#define can_bus_disable_terminator	can_bus_res = 0

/** D E F I N E ************************************/
#define CAN_BUFFER_SIZE_TX 2
#define CAN_BUFFER_SIZE_RX 2

/****************************************************
*	Filter/Mask truth table
*
*	Mask	Filter	Msg Id	Accept or
*							Reject bit
*
*	 0		  x		  x		  Accept
*	 1		  0		  0		  Accept
*	 1		  0		  1  	  Reject
*	 1		  1	  	  0		  Reject
*	 1		  1		  1		  Accept
****************************************************/
// Receive acceptance mask
#define MASK0_ID	0b000000000000000000000000
#define MASK1_ID	0b000000000000000000000000

// Receive acceptance filter
#define FILTER0_ID		0b0000000000000000
#define FILTER1_ID		0b0000000000000000
#define FILTER2_ID		0b0000000000000000
#define FILTER3_ID		0b0000000000000000
#define FILTER4_ID		0b0000000000000000
#define FILTER5_ID		0b0000000000000000
#define FILTER6_ID		0b0000000000000000
#define FILTER7_ID		0b0000000000000000
#define FILTER8_ID		0b0000000000000000
#define FILTER9_ID		0b0000000000000000
#define FILTER10_ID		0b0000000000000000
#define FILTER11_ID		0b0000000000000000
#define FILTER12_ID		0b0000000000000000
#define FILTER13_ID		0b0000000000000000
#define FILTER14_ID		0b0000000000000000
#define FILTER15_ID		0b0000000000000000

//define how to use the programmable buffer
#define CAN_PROG_MODE CAN_PROG_Bn_RX

// define which interrupts enable
#define CAN_CONFIG CAN_INTERRUPT_TX_ALL_ON | CAN_INTERRUPT_RX_ALL_ON | CAN_INTERRUPT_ERROR_OFF
#define CAN_CONFIG_PROG CAN_INTERRUPT_PROG_ALL_ON | CAN_INTERRUPT_RX_ALL_ON

// Intrruupt enable flags
#define CAN_INTERRUPT_TX	(CAN_CONFIG & CAN_INTERRUPT_TX_ALL_ON)
#define CAN_INTERRUPT_RX	(CAN_CONFIG & CAN_INTERRUPT_RX_ALL_ON)

/** S T R U C T  ***********************************/
extern struct CAN_STATUS
{
  unsigned buffer_tx_full 		:1;	// tx buffer full
  unsigned buffer_tx_empty 		:1; // tx buffer empty
  unsigned buffer_rx_full		:1; // rx buffer full
  unsigned buffer_rx_empty		:1; // rx buffer empty
  unsigned buffer_rx_overflow	:1; // rx buffer full and uart still receive data
  unsigned buffer_rx_error		:1;
};

/** S H A R E D  V A R I A B L E  ******************/
extern struct CAN_STATUS can_status;	// can's status flag

extern CAN_MESSAGE can_buffer_tx[CAN_BUFFER_SIZE_TX];	//tx buffer
extern CAN_MESSAGE can_buffer_rx[CAN_BUFFER_SIZE_RX];	//rx buffer


/** P R O T O T Y P E S ****************************/
unsigned char can_init(void);					// initialization of uart module*/

// puts data in tx buffer
unsigned char can_buffer_tx_load(CAN_MESSAGE *data_write);

// send the message store in tx buffer over can bus
unsigned char can_buffer_send(unsigned char can_buffer);

// returns size of empty section in tx buffer
unsigned char can_get_tx_buffer_empty_space(void);

// read a message from can
unsigned char can_buffer_rx_load(void);

// returns the message from rx buffer	
unsigned char can_buffer_read(CAN_MESSAGE *data_read);

// called from ISR from the main program
void can_isr(void);

#endif
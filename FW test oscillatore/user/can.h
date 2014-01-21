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

#ifndef _CAN_H
#define _CAN_H

/** I N C L U D E S ********************************/

/** D E F I N E ************************************/

// Operational mode
#define CAN_CONFIGURATION_OP 	4
#define CAN_LISTEN_ONLY_OP	 	3
#define CAN_LOOPBACK_OP			2
#define CAN_DISABLE_SLEEP_OP 	1
#define CAN_NORMAL_OP			0

// for description of modes see cap. 27.4
#define LEGACY_MODE	 			0
#define ENHANCED_LEGACY_MODE	1
#define ENHANCED_FIFO_MODE		2

// Flags to mask id message
#define CAN_ALL_MESSAGE_ACCEPTED	0
#define CAN_ONLY_EXID_ACCEPTED		1

// Configuration bit masks to be 'ored' together and passed as the parameter to
// the 'can_init_filter_buffer' routine. This can be used also as param
// in buffer_tx_ptr_add() function 
#define B0		0
#define B1		1
#define B2		2
#define B3		3
#define B4		4
#define B5		5
#define RX_B0	6
#define RX_B1	7
#define TX_B0 	6
#define TX_B1	7
#define TX_B2	8

/** S H A R E D  V A R I A B L E S  ****************/

/** E N U M  ***************************************/
// enum to anded to select flags
enum CAN_RX_FLAGS
{
  CAN_RX_FILTER_BITS 	= 0b00001111,

  CAN_RX_FILTER1		= 0b00000000,
  CAN_RX_FILTER2		= 0b00000001,
  CAN_RX_FILTER3		= 0b00000010,
  CAN_RX_FILTER4		= 0b00000011,
  CAN_RX_FILTER5		= 0b00000100,
  CAN_RX_FILTER6 		= 0b00000101,
  CAN_RX_FILTER7		= 0b00000110,
  CAN_RX_FILTER8		= 0b00000111,
  CAN_RX_FILTER9		= 0b00001000,
  CAN_RX_FILTER10		= 0b00001001,
  CAN_RX_FILTER11		= 0b00001010,
  CAN_RX_FILTER12 		= 0b00001011, 
  CAN_RX_FILTER13		= 0b00001100,
  CAN_RX_FILTER14		= 0b00001101,
  CAN_RX_FILTER15		= 0b00001110,
  CAN_RX_FILTER16 		= 0b00001111,

  CAN_RX_OVERFLOW 		= 0b00010000,

  CAN_RX_INVALID_MSG	= 0b00100000,

  CAN_RX_XTD_FRAME		= 0b01000000,

  CAN_RX_RTR_FRAME		= 0b10000000
};

/** S T R U C T  ***********************************/
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

// List of RBnCON register. This is necessary due to the Silicon Errata 
// (DS80519D) (see pag. 6)
extern rom struct 
{
  unsigned char *RXBnCON[8];

} RXBn_list;

extern rom struct 
{
  unsigned char *TXBnCON[9];

} TXBn_list;

/** V A R I A B L E  *******************************/
typedef struct CAN_MESSAGE_STRUCT CAN_MESSAGE;
extern rom struct RXBn_list;
extern rom struct TXBn_list;

/** P R O T O T Y P E S ****************************/
// Initialization for can's interrupts
void can_init_interrupt(unsigned char mode, unsigned char config_interrupt, 
						unsigned char config_prog_interrupt);

// select programmable buffer to be used as tx buffer or rx buffer
void can_init_prog_buffer_mode(unsigned char prog_buffer_mode);

// initialize mask				
void can_init_mask_id(unsigned char mask, unsigned long mask_id, 
					  unsigned char exiden);

// enable selected filter
void can_init_filter_enable(unsigned int filter);

//associate filter to an rx buffer
unsigned char can_init_filter_buffer(unsigned char filter, unsigned char buffer);
// associate filter to mask
void can_init_filter_mask(unsigned char filter, unsigned char mask);
// initialize filter ID
void can_init_filter_id(unsigned char filter, unsigned long filter_id, unsigned char exiden);

// initialize baudrate and bit timing
unsigned char can_init_baudrate_set(unsigned char SJW, unsigned char PRSEG,
									unsigned char PS1, unsigned char PS2, 
									unsigned char brp);

void can_op_select(unsigned char op);	// select a new operational mode
// select one of the three mode available: see define
unsigned char can_mode_select(unsigned char mode);	

//read message from buffer
void can_read(CAN_MESSAGE *can_message, unsigned char buffer_index, unsigned char release_now);

//release rx buffers
void can_buffer_rx_release(unsigned char buffer_mask);

//send the message over the bus
unsigned char can_write(unsigned char buffer);

unsigned char can_load_buffer(unsigned char buffer, CAN_MESSAGE *message);

//Search the first rx buffer that received data
unsigned char can_search_buffer_loaded(void);

// set the message priority
void can_buffer_set_priority(unsigned char buffer_index, unsigned char priority);

#endif // _CAN_H
/***************************************************
* FileName:        can_interface.c
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
* First written on 31/10/2012 by Luca Lucci.
*
* Module Description:
* This module provide functions for can bus interface. 
*
* Here has been decide to use large structure as TXBn_list or RXFn_list to 
* delete long switch..case structure. This structure has been stored in program
* memory, saving memory up to 1500 bits of program memory and 252 bit of data
* memory respect the first implementation.
*
* TODO:
*	insert input argument in can_init such as prog buffer rx/tx selection
*
****************************************************/

/** I N C L U D E S ********************************/
#include "can_interface.h"
#include "can.h"
#include "../system/io_cfg.h"	// definition of can bus pins
#include <p18cxxx.h>
#include <string.h>	//to use memcpy
#include "user\uart_interface.h"	// for test only. remove

/** S T R U C T  ***********************************/
struct CAN_STATUS
{
  unsigned buffer_tx_full 		:1;	// tx buffer full
  unsigned buffer_tx_empty 		:1; // tx buffer empty
  unsigned buffer_rx_full		:1; // rx buffer full
  unsigned buffer_rx_empty		:1; // rx buffer empty
  unsigned buffer_rx_overflow	:1; // rx buffer full and can still receive data
  unsigned buffer_rx_error		:1; // rx message error
};

/** V A R I A B L E  *******************************/
struct CAN_STATUS can_status;	// can's status flag

CAN_MESSAGE can_buffer_tx[CAN_BUFFER_SIZE_TX];	//tx buffer
unsigned char can_buffer_tx_data_cnt;	// number of byte to transmit
unsigned char can_buffer_tx_wr_ptr;	// write position in tx buffer
unsigned char can_buffer_tx_rd_ptr;	// read position to place data from

CAN_MESSAGE can_buffer_rx[CAN_BUFFER_SIZE_RX];	//rx buffer
unsigned char can_buffer_rx_data_cnt;	// number of byte received
unsigned char can_buffer_rx_wr_ptr;	// write position in rx buffer
unsigned char can_buffer_rx_rd_ptr;	// read position

//mask to recognize tx buffer. The least 3 bits are always true (TX0, TXB1 and 
//TXB2)
unsigned int can_buffer_tx_mask;
volatile unsigned int can_buffer_tx_free; //flag that indicate the buffer tx free

unsigned int count;		// for test only: remove

/**************************************************
* Function name		: unsigned char can_init(unsigned char global_interrupt, unsigned char prog_interrupt,
					   						unsigned char prog_buffer_mode, unsigned char baud_rate)
* 	result			: 0 - failure
*					: 1 - success
*   global_interrupt: which interrupt use (tx,rx,etc)
*	prog_interrupt	: which programmable buffer's interrupt to be enabled
* 	prog_buffer_mode: select if a programmable buffer is associate to tx or rx
*					  driver. For flags, see can_interface.h
*	baud_rate		: select desired baud rate. Use the follow parameters:
*						3: 1000kbps       
*						2: 500kbps      
*						1: 250kbps       
*						0: 125kbps   
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function initialize can peripheral. This will be 
*					  configured in mode1 - enhanced legacy mode.
* Notes				: This function initialize mask and filter to a default value!
**************************************************/
unsigned char can_init(unsigned char global_interrupt, unsigned char prog_interrupt,
					   unsigned char prog_buffer_mode, unsigned char baud_rate)
{
  unsigned char i;	//variable for loop

  count = 0x0000;
  //Init status flag
  can_status.buffer_tx_full = 0;
  can_status.buffer_tx_empty = 1;
  can_buffer_tx_data_cnt = 0;
  can_buffer_tx_wr_ptr = 0;
  can_buffer_tx_rd_ptr = 0;

  can_status.buffer_rx_full = 0;
  can_status.buffer_rx_empty = 1;
  can_status.buffer_rx_overflow = 0;
  can_status.buffer_rx_error = 0;
  can_buffer_rx_data_cnt = 0;
  can_buffer_rx_wr_ptr = 0;
  can_buffer_rx_rd_ptr = 0;

  // There's 3 tx buffer for sure and they are free on startup
  can_buffer_tx_mask = 0b00000111;
  can_buffer_tx_free = 0b00000111;

  // Init pins for canbus
  can_bus_tx_tris = OUTPUT_PIN;     // can bus' tx pin
  can_bus_rx_tris = INPUT_PIN;		// can bus' rx pin
  can_bus_res_tris = OUTPUT_PIN;	// enable pin for 120 ohm terminator

  // Disable terminator
  can_bus_disable_terminator;

  // Ensure that can module is in configuration mode
  can_op_select(CAN_CONFIGURATION_OP);
    
  // Select ECAN operational mode to enhanced legacy mode with three transmit
  // buffers, two receive buffers, six buffers programmable as TX or RX, 
  // automatic RTR handle, sixteen dynamic assigned acceptance filters, two
  // acceptance mask registers,	programmable data filter on standard identifier
  // messages.
  if( !can_mode_select(ENHANCED_LEGACY_MODE) )
  {
    can_op_select(CAN_NORMAL_OP);
    return 0;
  }

  // Select if use programmable buffers as tx or rx ones
  can_init_prog_buffer_mode(prog_buffer_mode);

  // Update buffer mask
  can_buffer_tx_mask = (BSEL0 << 1) | 0x07;
  can_buffer_tx_free = can_buffer_tx_mask;

  // Configure interrupts
  can_init_interrupt(global_interrupt, prog_interrupt);

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
  // Initialize Masks to accept all message
  can_init_mask_id(0, 0b000000000000000000000000, CAN_ALL_MESSAGE_ACCEPTED);
  can_init_mask_id(1, 0b000000000000000000000000, CAN_ALL_MESSAGE_ACCEPTED);

  // Initialize filters to accept all message with standard id
  for(i = 0; i < 16; i++)
  	can_init_filter_id(i, 0b0000000000000000, 0);

  // Enable/disable buffer filters through RXFENn in RXFCONn
  can_init_filter_enable(RXF_ALL_OFF);

  // Associate filters to rx buffers. In this case init 2 filter for each
  // buffer
  for( i = 0; i < 16; i++)
  {
    can_init_filter_buffer(i, i >> 1);
  }

  // Association buffer filters to acceptance mask. I want 1 mask for each
  // one half filter
  for( i = 0; i < 16; i++)
  {
    can_init_filter_mask(i, i >> 3);
  }

  // Configure bit timing												 
  if(!can_init_baudrate_set(1, 3, 2, 2, FOSC_MHZ, baud_rate))
  {
    can_op_select(CAN_NORMAL_OP);
    return 0;
  }

  // Set in normal state
  can_op_select(CAN_NORMAL_OP);

  return 1;
}

/**************************************************
* Function name		: unsigned char can_buffer_tx_load(unsigned char *data_write)
*	return			: 0 = failure - the buffer is full or invalid message
*					: 1 = success - buffer loaded
*	data_write		: pointer to the data to load in the tx buffer
* Created by		: Luca Lucci
* Date created		: 04/11/12
* Description		: This function puts the data into tx buffer. It place the
*                	  argument data_write in transmit buffer and updates the data
*  					  count and write pointer variables.
* Notes				: The interrupt will be disable until the end.
**************************************************/
unsigned char can_buffer_tx_load(CAN_MESSAGE *data_write)
{	

  // Check if the buffer is full; if not, add one byte of data.
  // If buffer is full assert queue flag to indicate that a 
  // message is waiting to write
  if(can_status.buffer_tx_full)
    return 0;

  // copy message info into tx buffer
  can_buffer_tx[can_buffer_tx_wr_ptr].id = data_write->id;
  can_buffer_tx[can_buffer_tx_wr_ptr].exid_frame = data_write->exid_frame;
  can_buffer_tx[can_buffer_tx_wr_ptr].rtr_frame = data_write->rtr_frame;

  if(data_write->data_length < 9)
    can_buffer_tx[can_buffer_tx_wr_ptr].data_length = data_write->data_length;

  memcpy((unsigned char *)&can_buffer_tx[can_buffer_tx_wr_ptr].data, &data_write->data, data_write->data_length);

  // update status data
  can_status.buffer_tx_empty = 0;
  can_buffer_tx_data_cnt++;
  
  if( can_buffer_tx_data_cnt == CAN_BUFFER_SIZE_TX)
    can_status.buffer_tx_full = 1;

  can_buffer_tx_wr_ptr++;

  if( can_buffer_tx_wr_ptr == CAN_BUFFER_SIZE_TX)
    can_buffer_tx_wr_ptr = 0;

  return 1;
}


/**************************************************
* Function name		: void can_buffer_send(void)
*	return			: 0 - failure: fail to send the message
*					: 1 - success
* Created by		: Luca Lucci
* Date created		: 11/11/12
* Description		: This function sends the data from tx buffer to 
*					  CAN buffer and updates the data count and read pointer 
*					  variables of transmit buffer. When interrupt is disable,
*					  the can_buffer param should be -1
* Notes				: if interrupt is disable, this is a blocking function!
*					  It wait until one buffer frees.
**************************************************/
unsigned char can_buffer_send(void)
{
  unsigned char buffer_index;	//buffer's index where to send the data
  unsigned char i;	// for loop

  //to remember when interrupt has to be enabled
  unsigned char txbnie_flag = 0;

  // check if tx buffer is empty
  if( !can_status.buffer_tx_empty )
  {

#ifndef CAN_INTERRUPT_TX
	// searching for a free tx buffer
    do{
      buffer_index = can_load_buffer(-1, &can_buffer_tx[can_buffer_tx_rd_ptr]);
    } while( buffer_index == -1 );
#else

    // else load a free buffer that updates with interrupt
    if(can_buffer_tx_free)
    {
      //critical code, disable interrupt
	  if(PIE5bits.TXBnIE)
	  {
	    PIE5bits.TXBnIE = 0;
		txbnie_flag = 1;
	  }
	  else
		txbnie_flag = 0;

      //find the buffer with highest priority
      for(i = 9; i > 0; i--)
      {
        if(((can_buffer_tx_free & can_buffer_tx_mask) >> (i - 1)) & 0x01)
        {
	      buffer_index = i - 1;
          can_buffer_tx_free &= ~(0x001 << buffer_index);	//reset flag for buffer
	      break;
        }
      }

	  // re-enable interrupt
	  if(txbnie_flag)
	    PIE5bits.TXBnIE = 1;

	  buffer_index = can_load_buffer(buffer_index, &can_buffer_tx[can_buffer_tx_rd_ptr]);

      if(buffer_index == -1) // no can buffer availble
        return 0;
#endif

    if(!can_write(buffer_index))
      return 0;
	else
	  count++;

    if(can_status.buffer_tx_full)
      can_status.buffer_tx_full = 0;

    can_buffer_tx_data_cnt--;

    if(can_buffer_tx_data_cnt == 0 )
      can_status.buffer_tx_empty = 1;

    can_buffer_tx_rd_ptr++;

    if(can_buffer_tx_rd_ptr == CAN_BUFFER_SIZE_TX)
      can_buffer_tx_rd_ptr = 0;

  }

#ifdef CAN_INTERRUPT_TX
  }
#endif
  
  return 1;

}

/**************************************************
* Function name		: unsigned char can_get_tx_buffer_empty_space(void)
*	return			: 0 = no space left
*					  unsigned char = number of free byte in tx buffer
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function returns the number of bytes of free space 
*					  left out in transmit buffer at the calling time. It 
*					  helps the user to further write data into trasmit buffer
*					  at once, rather than checking transmit buffer.
* Notes				: -
**************************************************/
unsigned char can_get_tx_buffer_empty_space(void)
{
  if( can_buffer_tx_data_cnt < CAN_BUFFER_SIZE_TX)
    return( CAN_BUFFER_SIZE_TX - can_buffer_tx_data_cnt);
  else
    return 0;
}

/**************************************************
* Function name		: unsigned char can_buffer_rx_load(void)
*	return			: 0 = failure
*					  1 = success
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function load the data from the can buffer receiver to 
*					  the read buffer. It also check error states.
* Notes				: If interrupt has been disabled, this function polling 
*    				  RCIF bit. If it's clear no data will be storage in 
*					  receive buffer
**************************************************/
unsigned char can_buffer_rx_load(void)
{
  unsigned char buffer_index;

  if(!can_status.buffer_rx_full)
  {

#ifdef CAN_INTERRUPT_RX
	buffer_index = CANSTAT & 0x0F;
#else
    buffer_index = can_search_buffer_loaded();

    if(buffer_index == -1)
      return 0;
#endif

    can_read(&can_buffer_rx[can_buffer_rx_wr_ptr], buffer_index);

    can_status.buffer_rx_overflow = 0;
    can_status.buffer_rx_empty = 0;

    //update status
    can_buffer_rx_data_cnt++;
      
    if( can_buffer_rx_data_cnt == CAN_BUFFER_SIZE_RX )
      can_status.buffer_rx_full = 1;

    can_buffer_rx_wr_ptr++;

    if( can_buffer_rx_wr_ptr == CAN_BUFFER_SIZE_RX )
      can_buffer_rx_wr_ptr = 0;
  }
  else
  {
	// Disable RX interrupt due buffer overflow. I will be re-enable when
	// overflow flags has been cleared
	PIE5bits.RXBnIE = 0;

    return 0;
  }

  return 1;
}

/**************************************************
* Function name		: unsigned char can_buffer_read(CAN_MESSAGE *data_read)
*	return			: 0 = buffer empty
*					  1 = data read correctly
*	data_read		: structure pointer where store the data from rx buffer
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function reads the data from the receive buffer. It
*					  places the data in to argument and updates the data count
*					  and read pointer
* Notes				: In this function the interrupt will be disabled, so it 
*					  can possible keeps the access pointer values proper.
**************************************************/
unsigned char can_buffer_read(CAN_MESSAGE *data_read)
{
  if(can_status.buffer_rx_empty)
    return 0;

  // critical code: disabling interrupts here keeps the access pointer values 
  // proper
#ifdef CAN_INTERRUPT_RX
  PIE5bits.RXBnIE = 0;
#endif 

  can_status.buffer_rx_full = 0;

  // load message
  data_read->id = can_buffer_rx[can_buffer_rx_rd_ptr].id;
  data_read->exid_frame = can_buffer_rx[can_buffer_rx_rd_ptr].exid_frame;
  data_read->rtr_frame = can_buffer_rx[can_buffer_rx_rd_ptr].rtr_frame;

  data_read->data_length = can_buffer_rx[can_buffer_rx_rd_ptr].data_length;

  data_read->filter_hit = can_buffer_rx[can_buffer_rx_rd_ptr].filter_hit;

  memcpy((unsigned char *)&data_read->data, &can_buffer_rx[can_buffer_rx_rd_ptr].data, can_buffer_rx[can_buffer_rx_rd_ptr].data_length);

  can_buffer_rx_data_cnt--;

  if(can_buffer_rx_data_cnt == 0)
    can_status.buffer_rx_empty = 1;

  can_buffer_rx_rd_ptr++;

  if(can_buffer_rx_rd_ptr == CAN_BUFFER_SIZE_RX)
    can_buffer_rx_rd_ptr = 0;

#ifdef CAN_INTERRUPT_RX
  PIE5bits.RXBnIE = 1;
#endif

  return 1;
}

/**************************************************
* Function name		: unsigned char uart1_get_rx_data_size(void);
*	return			: number of byte to read through uart_buffer_rx_load
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function returns the number of bytes of data 
*					  available in receive buffer at the calling time. It helps
					  the user to read data from receive buffer at once.
* Notes				: -
**************************************************/
unsigned char can_get_rx_data_size(void)
{
  return can_buffer_rx_data_cnt;
}

/**************************************************
* Function name		: void can_isr(void)
* 
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function handle isr 
* Notes				: -
**************************************************/
void can_isr(void)
{
  // temporary variable to prevent buffer mapping corruption between different
  // interrupts
  unsigned char EWIN_temp;	

#ifdef CAN_INTERRUPT_TX
  unsigned char buffer_index;
#endif

#ifdef CAN_INTERRUPT_TX
  // This interrupt can't be used as uart due to different meaning of interrupt flag
  if(PIR5bits.TXBnIF && PIE5bits.TXBnIE)
  {
    PIE5bits.TXBnIE = 0;  

    //One or more transmit buffers have completed transmission of a message and
	// may be reloaded 
    // get the buffer that cause the interrupt
	switch(CANSTAT & 0x1F)
    {
      case 4:	//TXB2
		buffer_index = 2;
		break;
	  case 6:	//TXB1
		buffer_index = 1;
		break;
	  case 8:	//TXB0
		buffer_index = 0;
		break;
	  case 18:	//TXB0
	  case 19:	//TXB1	
	  case 20:	//TXB2
	  case 21:	//TXB3
	  case 22:	//TXB4
	  case 23:	//TXB5
		buffer_index = (CANSTAT & 0x0F) + 1;
		break;
    }

	// Due to the silicon errata I have to use the
	// structure RXBn_list
	// reset interrupt flags. 
    *TXBn_list.TXBnCON[buffer_index] &= 0x7F;

    PIR5bits.TXBnIF = 0;  

    can_buffer_tx_free |= (0x001 << buffer_index);	//set flag for free buffer

    PIE5bits.TXBnIE = 1;

  }
#endif

#ifdef CAN_INTERRUPT_RX
  // Message received
  if(PIR5bits.RXBnIF && PIE5bits.RXBnIE)
  {
	EWIN_temp = CANSTAT & 0x0F;

    //read the buffer that had received the new data
    can_buffer_rx_load();

	// Due to the silicon errata I have to use the
	// structure RXBn_list
    // Clear RXFUL flag, so it can be possible to receive another message
    *RXBn_list.RXBnCON[EWIN_temp] &= 0x7F;

    //Clear the received flag. This be done after clear RXFUL bit!
    PIR5bits.RXBnIF = 0;

  }
#endif

  //Error accurs during trasmission or reception of a message
  if(PIR5bits.IRXIF && PIE5bits.IRXIE)
  {
    PIR5bits.IRXIF = 0;
	can_status.buffer_rx_error = 1;
    //led_st3 = 1;
  }

  // Error due to overflow condition or error state of the trasmitter or
  // receiver has changed
  if(PIR5bits.ERRIF && PIE5bits.ERRIE)
  {
    PIR5bits.ERRIF = 0;
    can_status.buffer_rx_overflow = 1;
    //led_st3 = 1;

	COMSTATbits.RXB1OVFL = 0; // reset overflow flag
  }

}
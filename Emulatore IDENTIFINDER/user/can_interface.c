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
*
****************************************************/

/** I N C L U D E S ********************************/
#include "can_interface.h"
#include "can.h"
#include "../system/io_cfg.h"	// definition of can bus pins
#include <p18cxxx.h>
#include <string.h>	//to use memcpy
#include <stdio.h>  //to use sprintf
//#include "user\uart_interface.h"	// for test only. remove

/** S T R U C T  ***********************************/
struct CAN_STATUS
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

  unsigned error_invalid_baudrate		:1; // invalid baud rate selected
  unsigned error_invalid_message		:1; // try to send an invalid message
};

#define can_buffer_rx_mask ((~can_buffer_tx_mask >> 1) | 0x3)

/** V A R I A B L E  *******************************/
struct CAN_STATUS can_status;	// can's status flag

CAN_MESSAGE can_buffer_tx[CAN_BUFFER_SIZE_TX];	//tx buffer
unsigned char can_buffer_tx_data_cnt;	// number of byte to transmit
unsigned char can_buffer_tx_wr_ptr;	// write position in tx buffer
unsigned char can_buffer_tx_rd_ptr;	// read position to place data from

#pragma udata CAN_REC
CAN_MESSAGE can_buffer_rx[CAN_BUFFER_SIZE_RX];	//rx buffer
#pragma udata

unsigned char can_buffer_rx_data_cnt;	// number of byte received
unsigned char can_buffer_rx_wr_ptr;	// write position in rx buffer
unsigned char can_buffer_rx_rd_ptr;	// read position

//mask to recognize tx buffer. The least 3 bits are always true (TX0, TXB1 and 
//TXB2)
unsigned int can_buffer_tx_mask;
volatile unsigned int can_buffer_tx_free; //flag that indicate the buffer tx free
volatile unsigned char can_buffer_rx_loaded; //flag to indicate that a buffer rx has been loaded

/** P R I V A T E  P R O T O T Y P E S ****************************/
void can_buffer_tx_free_update(void);
void can_buffer_rx_loaded_update(void);

/**************************************************
* Function name		: unsigned char can_init(unsigned char global_interrupt, unsigned char prog_interrupt,
*					   						unsigned char prog_buffer_mode, unsigned char baud_rate)
* 	result			: 0 - failure
*					: 1 - success
*   global_interrupt: which interrupt use (tx,rx,etc)
*	prog_interrupt	: which programmable buffer's interrupt to be enabled
* 	prog_buffer_mode: select if a programmable buffer is associate to tx or rx
*					  driver. For flags, see can_interface.h
*	sync_param		: array made of 5 byte that reflects the param for can:
*					  param0:	swj
*					  param1: 	prop_seg
*					  param2:   ps1
*					  param3:   ps2
*					  param4:	brp
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function initialize can peripheral. This will be 
*					  configured in mode1 - enhanced legacy mode.
* Notes				: This function initialize mask and filter to a default value!
**************************************************/
unsigned char can_init(unsigned char global_interrupt, unsigned char prog_interrupt,
					   unsigned char prog_buffer_mode, unsigned char* sync_param)
{
  unsigned char i;	//variable for loop

  //Init status flag
  can_status.buffer_tx_full = 0;
  can_status.buffer_tx_empty = 1;
  can_status.buffer_tx_error_warning = 0;
  can_status.buffer_tx_error_passive = 0;
  can_status.buffer_tx_error_off = 0;

  can_buffer_tx_data_cnt = 0;
  can_buffer_tx_wr_ptr = 0;
  can_buffer_tx_rd_ptr = 0;

  can_status.buffer_rx_full = 0;
  can_status.buffer_rx_empty = 1;
  can_status.buffer_rx_error_overflow = 0;
  can_status.buffer_rx_error_generic = 0;
  can_status.buffer_rx_error_warning = 0;
  can_status.buffer_rx_error_passive = 0;

  can_buffer_rx_data_cnt = 0;
  can_buffer_rx_wr_ptr = 0;
  can_buffer_rx_rd_ptr = 0;

  can_status.error_invalid_baudrate = 0;
  can_status.error_invalid_message = 0;
 
  // There are 3 tx buffer for sure and they are free on startup
  can_buffer_tx_mask = 0x0007;
  can_buffer_tx_free = 0x0007;

  // There are 2 rx buffer for sure and they are free on startup
  can_buffer_rx_loaded = 0x00;

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
  // bsel0 register show which configurable register are set in tx or rx mode.
  // I add the first 3 '1' to the word that rapresent the fix tx buffer. To do 
  // so I need to shift BSEL0 on the left, thus I cannot see the last configurable
  // register. I will not use it
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
  if(!can_init_baudrate_set(sync_param[0], sync_param[1], sync_param[2], sync_param[3], sync_param[4]))
  {
    can_status.error_invalid_baudrate = 1;
    can_op_select(CAN_NORMAL_OP);
    return 0;
  }

  // Set in normal state
  can_op_select(CAN_NORMAL_OP);
  //can_op_select(CAN_LOOPBACK_OP);

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
  unsigned char i;	//for loop

  // Check if the buffer is full; if not, add one byte of data.
  // If buffer is full assert queue flag to indicate that a 
  // message is waiting to write
  if(can_status.buffer_tx_full)
    return 0;

  // Maximum data per frame is 8. Check for invalid frame data length
  if(data_write->data_length > 8)
  {
    can_status.error_invalid_message = 1;
    return 0;
  }

  // copy message info into tx buffer
  can_buffer_tx[can_buffer_tx_wr_ptr].id = data_write->id;
  can_buffer_tx[can_buffer_tx_wr_ptr].exid_frame = data_write->exid_frame;
  can_buffer_tx[can_buffer_tx_wr_ptr].rtr_frame = data_write->rtr_frame;

  can_buffer_tx[can_buffer_tx_wr_ptr].data_length = data_write->data_length;

  for(i = 0; i < data_write->data_length; i++)
    can_buffer_tx[can_buffer_tx_wr_ptr].data[i] = data_write->data[i];

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
  static unsigned char buffer_temp = 9;	//to remember previouse buffer used
  static unsigned char buffer_priority = 3;	//store the last priority assigned

  unsigned char buffer_index;	//buffer's index where to send the data
  unsigned char buf_test;	// for loop

  //to remember when interrupt has to be enabled
  unsigned char txbnie_flag = 0;

  // check if tx buffer is empty
  if(!can_status.buffer_tx_empty)
  {
    #ifdef CAN_INTERRUPT_TX
      //critical code, disable interrupt
	  if(PIE5bits.TXBnIE)
	  {
	    PIE5bits.TXBnIE = 0;
	    txbnie_flag = 1;
	  }
	  else
	    txbnie_flag = 0; 
    #else
      can_buffer_tx_free_update();
    #endif

    // else load a free buffer that updates with interrupt
    if(can_buffer_tx_free)
    {
      //I want to use the buffer as a circle one, so I have to remember the previouse
	  //so I can use the next. This is done due to the type of priority managment that
	  //send data based on the buffer number. To reuse the first buffer when a lower
	  //one is set I mark them as "low priority"

	  if(can_buffer_tx_free == can_buffer_tx_mask)
	    buffer_priority = 3;	// set to the max priority again

	  if(buffer_temp == 0)
      {
		//if I have arrived to the lowest priority I can't send message in the
		//right order, so I must wait that all message have been sent
		if(buffer_priority == 0)
		  return 0;
		
        //I restart from buffer with higher number. To send message in order,
		//if there's another buffer load, then decrease the priority.
	    buffer_temp = 9;
        
        buffer_priority--;
	  }

      for(buf_test = buffer_temp; buf_test > 0; buf_test--)
      {
        if((can_buffer_tx_free >> (buf_test - 1)) & 0x01)
        {
	      buffer_index = buf_test - 1;
		  buffer_temp = buffer_index;
          can_buffer_tx_free &= ~(0x001 << buffer_index);	//reset flag for buffer

		  // set the message priority
		  can_buffer_set_priority(buffer_index, buffer_priority);
	      break;
        }
      }

      buffer_index = can_load_buffer(buffer_index, &can_buffer_tx[can_buffer_tx_rd_ptr]);

      if(buffer_index == -1) // no can buffer availble
        return 0;

      if(!can_write(buffer_index))
        return 0;

      if(can_status.buffer_tx_full)
        can_status.buffer_tx_full = 0;

      can_buffer_tx_data_cnt--;

      if(can_buffer_tx_data_cnt == 0 )
        can_status.buffer_tx_empty = 1;

      can_buffer_tx_rd_ptr++;

      if(can_buffer_tx_rd_ptr == CAN_BUFFER_SIZE_TX)
        can_buffer_tx_rd_ptr = 0;

    }
	else
    {
      #ifdef CAN_INTERRUPT_TX
        // re-enable interrupt
        if(txbnie_flag)
          PIE5bits.TXBnIE = 1;
      #endif 

	  return 0;
    }

    #ifdef CAN_INTERRUPT_TX
      // re-enable interrupt
      if(txbnie_flag)
        PIE5bits.TXBnIE = 1;
    #endif 

  }

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
  static unsigned char buffer_temp = 0;
  unsigned char buf_test;
  unsigned char buffer_index;

  if(!can_status.buffer_rx_full)
  {
    
#ifdef CAN_INTERRUPT_RX
    can_buffer_rx_loaded |= (0x0001 << (CANSTAT & 0x0F));
#else
    can_buffer_rx_loaded_update();
#endif
    //I can't decide where to put the message received from the MAB, but I can 
    //decide the read order. The polling cycle must be faster then the capacity
    //of the other device to fill the buffer, otherwise, while I'm reading a mid
    //register, the next one can be filled. If there's a problem with this function
    //try to read 3 buffer at once a release only later.

    if(can_buffer_rx_loaded)
    {
      //start from lower buffer
      for(buf_test = buffer_temp; buf_test < 8; buf_test++)
      {
        if((can_buffer_rx_loaded >> buf_test) & 0x01)
        {
          buffer_index = buf_test;
          buffer_temp = buffer_index + 1;
          can_buffer_rx_loaded &= ~(0x001 << buffer_index);	//reset flag for buffer
			
		  //if there's no message to read in the other buffer then reset the temp
		  //buffer index
		  if(!(can_buffer_rx_loaded >> buf_test))
		    buffer_temp = 0;

	      break;
        }
      }
      can_read(&can_buffer_rx[can_buffer_rx_wr_ptr], buffer_index);
      can_status.buffer_rx_empty = 0;

      //update status
      can_buffer_rx_data_cnt++;

      if(can_buffer_rx_data_cnt == CAN_BUFFER_SIZE_RX)
        can_status.buffer_rx_full = 1;

      can_buffer_rx_wr_ptr++;

      if( can_buffer_rx_wr_ptr == CAN_BUFFER_SIZE_RX )
        can_buffer_rx_wr_ptr = 0;

    }
  }
  else
  {
	// Disable RX interrupt due buffer overflow. I will be re-enable when
	// overflow flags has been cleared
	PIE5bits.RXBnIE = 0;

    return 0;
  }

  #ifdef CAN_INTERRUPT_RX
	PIE5bits.RXBnIE = 1;
  #endif

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

    EWIN_temp = CANSTAT & 0x1F;
    //One or more transmit buffers have completed transmission of a message and
	// may be reloaded 
    // get the buffer that caused the interrupt
	switch(EWIN_temp)
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
		buffer_index = (EWIN_temp & 0x0F) + 1;
		break;
    }

	// Due to the silicon errata I have to use the
	// structure RXBn_list
	// reset interrupt flags. 
    *TXBn_list.TXBnCON[buffer_index] &= 0x7F;

    PIR5bits.TXBnIF = 0;  

    can_buffer_tx_free |= (0x0001 << buffer_index);	//set flag for free buffer

    PIE5bits.TXBnIE = 1;

  }
#endif

#ifdef CAN_INTERRUPT_RX
  // Message received
  if(PIR5bits.RXBnIF && PIE5bits.RXBnIE)
  {
    //read the buffer that had received the new data
    can_buffer_rx_load();

    //Clear the received flag. This be done after clear RXFUL bit!
    PIR5bits.RXBnIF = 0;
  }
#endif

  //Error accurs during trasmission or reception of a message
  if(PIR5bits.IRXIF && PIE5bits.IRXIE)
  {
    PIR5bits.IRXIF = 0;
	can_status.buffer_rx_error_generic = 1;
  }

  // Error due to overflow condition or error state of the trasmitter or
  // receiver has changed
  if(PIR5bits.ERRIF && PIE5bits.ERRIE)
  {
    PIR5bits.ERRIF = 0;

	// All if statement but RXB1OVFL are for debug use
	if(COMSTATbits.RXB1OVFL)  //Receive Buffer n Overflow
	{
	  can_status.buffer_rx_error_overflow = 1;
	  COMSTATbits.RXB1OVFL = 0; // reset overflow flag
	}
	else if(COMSTATbits.TXBO) //Transmitter error counter > 255 - Bus-Off
	  can_status.buffer_tx_error_off = 1;
	else if(COMSTATbits.TXBP) //Trasmitter error counter > 127 - Bus-Passive
	  can_status.buffer_tx_error_passive = 1;
	else if(COMSTATbits.RXBP) //Receiver error > 127 - Bus-Passive
	  can_status.buffer_rx_error_passive = 1;
    else if(COMSTATbits.TXWARN)  //Transmitter error counter > 95 - Warning
	  can_status.buffer_tx_error_warning = 1;
    else if(COMSTATbits.RXWARN)  //Receiver error counter > 35 - Warning
	  can_status.buffer_rx_error_warning = 1;
  }

}

/**************************************************
* Function name		: void can_buffer_tx_free_update(void)
* 
* Created by		: Luca Lucci
* Date created		: 03/02/13
* Description		: This function update the can_buffer_tx_free buffer to 
*					  reflect the buffer's status. It have to be used in a
*					  polling function when tx interrupt are disable
* Notes				: -
**************************************************/
void can_buffer_tx_free_update(void)
{
  unsigned char i;

  for(i = 0; i < 9; i++)
  {
    // if the buffer is a tx one then update the buffer flag
    if(can_buffer_tx_mask & (0x01 << i))
    {
	  // if TXREQ is set then we have to clear the result bit in can_buffer_tx_free
	  // to remember that I cannot use it
	  if(*TXBn_list.TXBnCON[i] & 0x08)
	  {
        can_buffer_tx_free &= ~(0x0001 << i);
	  }
	  else
		can_buffer_tx_free |= (0x0001 << i);
    }
  }
}


/**************************************************
* Function name		: void can_buffer_rx_loaded_update(void)
* 
* Created by		: Luca Lucci
* Date created		: 07/02/13
* Description		: This function update the can_buffer_rx_loaded buffer to 
*					  reflect the buffer's status. It have to be used in a
*					  polling function when tx interrupt are disable
* Notes				: -
**************************************************/
void can_buffer_rx_loaded_update(void)
{
  unsigned char i;

  for(i = 0; i < 8; i++)
  {
    // if the buffer is a rx one then update the buffer flag
    if(can_buffer_rx_mask & (0x01 << i))
    {
	  // if RXREQ is set then we have to set the result bit in can_buffer_rx_free
	  // to remember that I can read from it
	  if(*RXBn_list.RXBnCON[i] & 0x80)
	  {
        can_buffer_rx_loaded |= (0x0001 << i);
	  }
	  else
		can_buffer_rx_loaded &= ~(0x0001 << i);
    }
  }
}

/**************************************************
* Function name		: unsigned char can_error_handle(unsigned char *error_message, unsigned char *length)
* 	return			: 0 - no error occurred
*					  1 - error message loaded
*	buffer			: pointer to a buffer where store the error message
*   length			: char where to store the length of the message 
* Created by		: Luca Lucci
* Date created		: 11/02/13
* Description		: This function check if there has been error in can 
*					  communication and load the error message into the buffer
*					  passed as param. Also return the length of the message 
*					  loaded.
* Notes				: Only one error will be passed each time, so user must be
*					  poll this function until it returns 0. The if position
*					  decides the priority of the error message. The minimum 
*					  buffer must be 30 byte
**************************************************/
unsigned char can_error_handle(unsigned char *error_message, unsigned char *length)
{
  if(can_status.error_invalid_baudrate)
  {
    *length = sprintf(error_message, "Can Invalid baudrate\r\n");
    can_status.error_invalid_baudrate = 0;
  }
  else if(can_status.buffer_rx_error_overflow)
  {
    *length = sprintf(error_message, "CAN buffer rx overflow\r\n");
	can_status.buffer_rx_error_overflow = 0;
  }
  else if(can_status.buffer_tx_error_off)
  {
    *length = sprintf(error_message, "CAN buffer tx Bus-off\r\n");
    can_status.buffer_tx_error_off = 0;
  }
  else if(can_status.buffer_tx_error_passive)
  {
	*length = sprintf(error_message, "CAN buffer tx Bus-Passive\r\n");
    can_status.buffer_tx_error_passive = 0;
  }
  else if(can_status.buffer_rx_error_passive)
  {
    *length = sprintf(error_message, "CAN buffer rx Bus-Passive\r\n");
    can_status.buffer_rx_error_passive = 0;
  }
  else if(can_status.buffer_tx_error_warning)
  {
    *length = sprintf(error_message, "CAN buffer tx Warning\r\n");
    can_status.buffer_tx_error_warning = 0;
  }
  else if(can_status.buffer_rx_error_warning)
  {
    *length = sprintf(error_message, "CAN buffer rx Warning\r\n");
    can_status.buffer_rx_error_warning = 0;
  }
  else if(can_status.buffer_rx_error_generic)
  {
   *length = sprintf(error_message, "Can Error\r\n");
    can_status.buffer_rx_error_generic = 0;
  }
  else if(can_status.error_invalid_message)
  {
    *length = sprintf(error_message, "Invalid can msg\r\n");
    can_status.error_invalid_message = 0;
  }
  else
    return 0;

  return 1;
}
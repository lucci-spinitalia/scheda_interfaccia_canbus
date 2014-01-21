/***************************************************
* FileName:        can.c
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
* This module provide functions for can bus interface. 
*
* Here has been decide to use large structure as RXFn_list to 
* delete long switch..case structure.
*
* Due to the silicon errata (DS80519D) it needed to use RXBn_list structure
* to select the rigth register, because register from B0CON to B1CON don't
* will be mapped into memory using ECANCON.EWIN.
*
*
****************************************************/

/** I N C L U D E S ********************************/
#include "can.h"
#include "../system/io_cfg.h"	// definition of can bus pins
#include <p18cxxx.h>
#include "user\uart_interface.h"	// for test only. remove
#include <math.h>	// for pow 

/** D E F I N E  ***********************************/

/** S T R U C T U R E   ****************************/
// List of all buffer registers. This structure provide a simple way to 
// implements all the register need for transmittion, without needs to 
// long "switch case" structure.
struct CAN_MESSAGE_STRUCT
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

// List of all receiver filter register
rom struct 
{
  unsigned char *RXFnSIDH[16];

} RXFn_list = {{&RXF0SIDH, &RXF1SIDH, &RXF2SIDH, &RXF3SIDH, &RXF4SIDH, &RXF5SIDH, &RXF6SIDH, &RXF7SIDH,
				&RXF8SIDH, &RXF9SIDH, &RXF10SIDH, &RXF11SIDH, &RXF12SIDH, &RXF13SIDH, &RXF14SIDH, &RXF15SIDH}};

// List of RBnCON register. This is necessary due to the Silicon Errata 
// (DS80519D) (see pag. 6)
// WARNING!!! Don't call this structure after set ECANCONWIN because in this
// case the first register will not be the same and some function as
// can_search_buffer_loaded will not work correctly
rom struct 
{
  unsigned char *RXBnCON[8];

} RXBn_list = {{(unsigned char *)&RXB0CON, (unsigned char *)&RXB1CON, 
				(unsigned char *)&B0CON, (unsigned char *)&B1CON, 
				(unsigned char *)&B2CON, (unsigned char *)&B3CON, 
				(unsigned char *)&B4CON, (unsigned char *)&B5CON}};

rom struct 
{
  unsigned char *TXBnCON[9];

} TXBn_list = {{(unsigned char *)&TXB0CON, (unsigned char *)&TXB1CON, (unsigned char *)&TXB2CON, 
				(unsigned char *)&B0CON, (unsigned char *)&B1CON, 
				(unsigned char *)&B2CON, (unsigned char *)&B3CON, 
				(unsigned char *)&B4CON, (unsigned char *)&B5CON}};

/** V A R I A B L E S  ******************/

/** P R I V A T E   P R O T O T Y P E   ************/
//Search the first buffer that can load data 
unsigned char can_search_buffer_free(void); 

//store can id in the registers form
void canid_to_regs(unsigned char *buffer, unsigned long id, unsigned char exide);


/**************************************************
* Function name		: void can_init_interrupt(unsigned char config_interrupt, 
*											  unsigned char config_prog_interrupt)
*   mode:					0 - Legacy Mode
*						    1 - Enhanced Mode
*							2 - Enhanced FIFO Mode
* 	config_interrupt : 		enable can interrupt such as non programmable tx, 
*							rx, error and invalid message. See PIE5 and TXBIE 
*						    register.
*	config_prog_interrupt: choose which of six programmable buffer interrupt 
*						   enable. See BIE0
* Created by		: Luca Lucci
* Date created		: 21/10/12
* Description		: This function initialize can peripheral's interrupts.
* Notes				: -
**************************************************/
void can_init_interrupt(unsigned char mode, unsigned char config_interrupt, 
						unsigned char config_prog_interrupt)
{
  unsigned char old_op_mode = CANSTATbits.OPMODE;  //hold the current op mode

  //This function must be called with can module in configuration mode.
  if(CANSTATbits.OPMODE != CAN_CONFIGURATION_OP)
	can_op_select(CAN_CONFIGURATION_OP);

  switch(mode)
  {
    case 0: //Legacy Mode
      PIE5 = config_interrupt;
      break;

    case 1:
    case 2:
	  PIE5 |= (config_interrupt & 0b11100000);

	  if(config_interrupt & 0b00000011)
        PIE5bits.RXB1IE = 1;

      if(config_interrupt & 0b00011100)
        PIE5bits.TXB2IE = 1;

      TXBIE = config_interrupt;
      BIE0 = config_prog_interrupt | (config_interrupt & 0x03);
      break;

    default:
      break;
  } 


  // return to previouse op mode
  if(old_op_mode != CANSTATbits.OPMODE)
    can_op_select(old_op_mode);
}

/**************************************************
* Function name		: void can_init_prog_buffer_mode(unsigned char prog_buffer)
*	prog_buffer_mode: bit of BSEL0 that indicate the desired buffer
* Created by		: Luca Lucci
* Date created		: 21/10/12
* Description		: This function initialize can programmable buffer's mode
* Notes				: In mode 2 the first tx register block the fifo, so it 
*                     should start from last position
**************************************************/
void can_init_prog_buffer_mode(unsigned char prog_buffer_mode)
{
  unsigned char old_op_mode = CANSTATbits.OPMODE;  //hold the current op mode

  //This function must be called with can module in configuration mode.
  if(CANSTATbits.OPMODE != CAN_CONFIGURATION_OP)
	can_op_select(CAN_CONFIGURATION_OP);

  BSEL0 = prog_buffer_mode;

  // return to previouse op mode
  if(old_op_mode != CANSTATbits.OPMODE)
    can_op_select(old_op_mode);
}

/**************************************************
* Function name		: void can_init_mask_id(unsigned char mask, unsigned long mask_id, unsigned char exiden).
*	mask			: mask's number - 0 or 1
* 	mask_sid		: 29 bit mask value - if exiden = 0 mask value is 11 bit long
*	exidend			: flag for accept all message or extended id message only
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function initialize mask ID.
* Notes				: -
**************************************************/
void can_init_mask_id(unsigned char mask, unsigned long mask_id, unsigned char exiden)
{
  unsigned char old_op_mode = CANSTATbits.OPMODE;  //hold the current op mode

  //This function must be called with can module in configuration mode.
  if(CANSTATbits.OPMODE != CAN_CONFIGURATION_OP)
	can_op_select(CAN_CONFIGURATION_OP);

  switch(mask)
  {
    case 0:
      canid_to_regs((unsigned char *)&RXM0SIDH , mask_id, exiden);
	  break;
  
    case 1:
      canid_to_regs((unsigned char *)&RXM1SIDH , mask_id, exiden);
	  break;

    default:
      break;
  }

  // return to previouse op mode
  if(old_op_mode != CANSTATbits.OPMODE)
    can_op_select(old_op_mode);
}


/**************************************************
* Function name		: void can_init_filter_enable(unsigned int filter)
* 	filter			: 16 bit mask to enable filters
* Created by		: Luca Lucci
* Date created		: 22/10/12
* Description		: This function enable receive filter
* Notes				: -
**************************************************/
void can_init_filter_enable(unsigned int filter)
{
  unsigned char old_op_mode = CANSTATbits.OPMODE;  //hold the current op mode

  //This function must be called with can module in configuration mode.
  if(CANSTATbits.OPMODE != CAN_CONFIGURATION_OP)
	can_op_select(CAN_CONFIGURATION_OP);

  RXFCON0 = filter & 0x00FF;
  RXFCON1 = (filter >> 8) & 0x00FF;

  // return to previouse op mode
  if(old_op_mode != CANSTATbits.OPMODE)
    can_op_select(old_op_mode);
}

/**************************************************
* Function name		: unsigned char can_init_filter_buffer(unsigned char filter, unsigned char buffer)
*	return			: 0 - failure: prog buffer set as tx
*					  1 - success
* 	filter			: filter's number - 0 to 15
* 	buffer			: buffer to associate to filter argue
* Created by		: Luca Lucci
* Date created		: 22/10/12
* Description		: This function associate filter to an rx buffer
* Notes				: -
**************************************************/
unsigned char can_init_filter_buffer(unsigned char filter, unsigned char buffer)
{
  unsigned char old_op_mode = CANSTATbits.OPMODE;  //hold the current op mode

  //can't associate acceptance filter to programmable buffer set as tx
  if((BSEL0 & (0x01 << buffer)))
	return 0;

  //This function must be called with can module in configuration mode.
  if(CANSTATbits.OPMODE != CAN_CONFIGURATION_OP)
	can_op_select(CAN_CONFIGURATION_OP);

  switch(filter)
  {
    case 0:
	  RXFBCON0bits.F0BP = buffer;
	  break;
    case 1:
	  RXFBCON0bits.F1BP = buffer;
	  break;
    case 2:
	  RXFBCON1bits.F2BP = buffer;
	  break;
    case 3:
	  RXFBCON1bits.F3BP = buffer;
	  break;
    case 4:
	  RXFBCON2bits.F4BP = buffer;
	  break;
    case 5:
	  RXFBCON2bits.F5BP = buffer;
	  break;
    case 6:
	  RXFBCON3bits.F6BP = buffer;
	  break;
    case 7:
	  RXFBCON3bits.F7BP = buffer;
	  break;
    case 8:
	  RXFBCON4bits.F8BP = buffer;
	  break;
    case 9:
	  RXFBCON4bits.F9BP = buffer;
	  break;
    case 10:
	  RXFBCON5bits.F10BP = buffer;
	  break;
    case 11:
	  RXFBCON5bits.F11BP = buffer;
	  break;
    case 12:
	  RXFBCON6bits.F12BP = buffer;
	  break;    
	case 13:
	  RXFBCON6bits.F13BP = buffer;
	  break;
    case 14:
	  RXFBCON7bits.F14BP = buffer;
	  break;
    case 15:
	  RXFBCON7bits.F15BP = buffer;
	  break;
	default:
		break;
  }

  // return to previouse op mode
  if(old_op_mode != CANSTATbits.OPMODE)
    can_op_select(old_op_mode);

  return 1;
}

/**************************************************
* Function name		: void can_init_filter_mask(unsigned char filter, unsigned char mask)
* 	filter			: filter's number - 0 to 15
* 	mask			: mask to associate to filter argue
* Created by		: Luca Lucci
* Date created		: 22/10/12
* Description		: This function associate filter to mask
* Notes				: -
**************************************************/
void can_init_filter_mask(unsigned char filter, unsigned char mask)
{
  unsigned char old_op_mode = CANSTATbits.OPMODE;  //hold the current op mode

  //This function must be called with can module in configuration mode.
  if(CANSTATbits.OPMODE != CAN_CONFIGURATION_OP)
	can_op_select(CAN_CONFIGURATION_OP);

  switch(filter)
  {
    case 0:
	  MSEL0bits.FIL0 = mask;
	  break;
    case 1:
	  MSEL0bits.FIL1 = mask;
	  break;
    case 2:
	  MSEL0bits.FIL2 = mask;
	  break;
    case 3:
	  MSEL0bits.FIL3 = mask;
	  break;
    case 4:
	  MSEL1bits.FIL4 = mask;
	  break;
    case 5:
	  MSEL1bits.FIL5 = mask;
	  break;
    case 6:
	  MSEL1bits.FIL6 = mask;
	  break;
    case 7:
	  MSEL1bits.FIL7 = mask;
	  break;
    case 8:
	  MSEL2bits.FIL8 = mask;
	  break;
    case 9:
	  MSEL2bits.FIL9 = mask;
	  break;
    case 10:
	  MSEL2bits.FIL10 = mask;
	  break;
    case 11:
	  MSEL2bits.FIL11 = mask;
	  break;
    case 12:
	  MSEL3bits.FIL12 = mask;
	  break;    
	case 13:
	  MSEL3bits.FIL13 = mask;
	  break;
    case 14:
	  MSEL3bits.FIL14 = mask;
	  break;
    case 15:
	  MSEL3bits.FIL15 = mask;
	  break;
	default:
		break;
  }

  // return to previouse op mode
  if(old_op_mode != CANSTATbits.OPMODE)
    can_op_select(old_op_mode);
}
/**************************************************
* Function name		: void can_init_filter_id(unsigned char filter, unsigned long filter_id, unsigned char exiden)
*	filter			: filter's number - 0 or 15
*	exiden			: flag that enable exended id. If 0 than id it must be of 11 bit
* 	filter_id		: filter id. If it is a standard id it must be 11 bit lenght
* Created by		: Luca Lucci
* Date created		: 23/10/12
* Description		: This function initialize filter ID
* Notes				: -
**************************************************/
void can_init_filter_id(unsigned char filter, unsigned long filter_id, unsigned char exiden)
{
  unsigned char old_op_mode = CANSTATbits.OPMODE;  //hold the current op mode

  //This function must be called with can module in configuration mode.
  if(CANSTATbits.OPMODE != CAN_CONFIGURATION_OP)
	can_op_select(CAN_CONFIGURATION_OP);

  canid_to_regs(RXFn_list.RXFnSIDH[filter] , filter_id, exiden);

  // return to previouse op mode
  if(old_op_mode != CANSTATbits.OPMODE)
    can_op_select(old_op_mode);
}

/**************************************************
* Function name		: unsigned char can_init_baudrate_set(unsigned char SJW, unsigned char PRSEG,
														  unsigned char PS1, unsigned char PS2, 
														  unsigned char brp)
* 	result			: 0 - failure: parameters out of range
*					: 1 - success
*	SJW				: sjw value in Tq
*	PRSEG			: prseg value in Tq
*	PS1				: ps1 value in Tq
*	PS2				: ps2 value in Tq
*	brp  			: baudrate prescaler
*
* Created by		: Luca Lucci
* Date created		: 22/10/12
* Description		: 
* Notes				: The frequencies of the oscillators in the different nodes
* 					  must be coordinated in order to provide a system wide 
*   				  specified nominal bit time.This means that all 
*					  oscillators must have a Tosc that is an integral divisor 
*					  of Tq.
**************************************************/
unsigned char can_init_baudrate_set(unsigned char SJW, unsigned char PRSEG,
									unsigned char PS1, unsigned char PS2, 
									unsigned char brp)
{
  unsigned char old_op_mode = CANSTATbits.OPMODE;  //hold the current op mode

  // Nominal bit time is programmable from a minimum of 8 Tq to a maximum of 25
  // Tq. The minimum nominal bit time is 1 us.
 
  // NBR = Nominal bit rate = 1 /  Nominale bit time
  // #Tq = (Sync_Seg + Prop_Seg + Phase_Seg1 + Phase_Seg2)
  // Tq = NBT / #Tq
  // Tq (us) = (2 * (BRP + 1) / Fosc(MHz) 
  
  // Sync segment = 1Tq
  // 1 Tq < PropSeg < 8 Tq
  // 1 Tq < Phase1 < 8 Tq
  // 2 Tq < Phase2 < 8 Tq
  // 1 Tq < SJW < 4 Tq
  // The sampling point should be as late as possible or approximately 80% of
  // the bit time

  // Condition:
  //
  // PropSeg + PS1 >= PS2
  // PropSeg + PS1 >= Tdelay = 1 or 2 Tq
  // PS2 > SJW

  // check the value on timing variable
  if( PRSEG > 0 && PRSEG < 9 && PS1 > 0 && PS1 < 9 && PS2 > 1 && PS2 < 9 && SJW > 0 && SJW < 5)
  {
    if( !((PRSEG + PS1) >= PS2) || !(PS2 >= SJW))
      return 0;
  }
  else
    return 0;


  //This function must be called with can module in configuration mode.
  if(CANSTATbits.OPMODE != CAN_CONFIGURATION_OP)
	can_op_select(CAN_CONFIGURATION_OP);

  if(PS1 > 1)
    BRGCON2bits.SAM = 1;	// Bus sampled three times prior to the sample point
  else
    BRGCON2bits.SAM = 0;	// Bus sampled once at the sample point	

  BRGCON2bits.SEG2PHTS = 1;	// seg2phase freely programmable

  // Select the minimum number of Tq
  BRGCON1bits.SJW = SJW - 1;		
  BRGCON2bits.PRSEG = PRSEG - 1;	
  BRGCON2bits.SEG1PH = PS1 - 1;	
  BRGCON3bits.SEG2PH = PS2 - 1;	
  BRGCON1bits.BRP = brp;

  // return to previouse op mode
  if(old_op_mode != CANSTATbits.OPMODE)
    can_op_select(old_op_mode);

  return 1;	// success
}

/**************************************************
* Function name		: unsigned char can_auto_rtr(unsigned char prog_buffer, unsigned char filter)
*	result			: 0 - failure: invalid buffer of filter
*					  1 - success
* 	buffer			: number of programmable buffer that will send the response
*					  to the RTR - valid value 0 to 6
*	filter			: filter that will be associated to the buffer -
*					  valid value 0 to 15
* Created by		: Luca Lucci
* Date created		: 23/10/12
* Description		: This function perform automatic respond to predefined RTR
*					  handling by use the buffer. 
* Notes				: This function must be called in configuration mode.
*					  The arg buffer will be changed to tx buffer and the 
*					  filter will be associated with buffer. When update the 
*					  buffer check that TXREQ is clear, otherwise any write
*					  operation will be ignored.
**************************************************/
unsigned char can_init_auto_rtr(unsigned char prog_buffer, unsigned char filter)
{
  if(filter > 15 || !(BSEL0 & ((int) pow(2,(prog_buffer+2)))))
	return 0;
  else
  {
	// associate filter to buffer. Add 3 due to the fix tx buffer.
    can_init_filter_buffer(filter, prog_buffer + 3);
	
	// Set RTREN flag in TXBn register to 1
	// warning: this register point to another tx register
	*TXBn_list.TXBnCON[prog_buffer+3] |= 0x4;
  }

  return 1;
  
}

/**************************************************
* Function name		: unsigned char can_mode_select(unsigned char mode)
*	return			: 0 - Failure
*					  1 - Success
* 	mode			: next mode
* Created by		: Luca Lucci
* Date created		: 21/10/12
* Description		: This function select one of the three mode available: 
*						mode 0 - legacy mode
*						mode 1 - enhanced legacy mode
*						mode 2 - enhanced FIFO mode
* Notes				: This function must be called with can module in 
*					  configuration mode. For detail see cap. 27.4.
*					  Mode 0 not implemented yet
**************************************************/
unsigned char can_mode_select(unsigned char mode)
{
  if(mode == LEGACY_MODE || mode > 2)
    return 0;

  ECANCONbits.MDSEL	= mode;

  return 1;
}

/**************************************************
* Function name		: void can_op_select(unsigned char op)
* 	op				: next operational mode
* Created by		: Luca Lucci
* Date created		: 21/10/12
* Description		: This function select a new operational mode
*						Configuration
						Listen only
						Loopback
						Disable/sleep
						Normal
* Notes				: This is a blocking function! Wait until the can module
*					  enters in new op mode. For detail see cap. 27.3
**************************************************/
void can_op_select(unsigned char op)
{
  CANCONbits.REQOP = op;
  
  while( (CANSTATbits.OPMODE) != op );
}

/**************************************************
* Function name		: unsigned char can_search_buffer_free(void)
*	return			: >=0 :index of buffer loaded.
*					  -1: no free buffer
* Created by		: Luca Lucci
* Date created		: 24/10/12
* Description		: Search the first tx buffer that can load data 
* Notes				: This function map the selected buffer in TXB0xxx.
*					  The RXB0xxx buffer it should be restored once used the
*					  previous one.
**************************************************/
unsigned char can_search_buffer_free(void)
{
  unsigned char i; // variable for loop

  // Due to the priority, it be necessary search in buffer with higher number
  for(i = 8; i >= 0; i--)
  {
    // if it's a programmable buffer, check if it has been configured as Tx
    // buffer
    if((i <= 2) || ((i > 2) && (BSEL0 & (0x01 << (i-1)))))
    {
      if(!(*TXBn_list.TXBnCON[i] & 0x08))
        return i;
    }
  }

  // else return an error
  return -1;
}

/**************************************************
* Function name		: unsigned char can_search_buffer_loaded(void)
*	return			: <= 7: index of buffer that receive data
*					  -1: no free buffer had received data
* Created by		: Luca Lucci
* Date created		: 24/10/12
* Description		: Search the first rx buffer that received data
* Notes				: Bit RXFUL must be cleared in software after read the 
*					  message
**************************************************/
unsigned char can_search_buffer_loaded(void)
{
  unsigned char i; // variable for loop

  for(i = 0; i < 8; i++)
  {
    // if it's a programmable buffer, check if it has been configured as rx
    // buffer
    if((i > 1) && (BSEL0 & (0x01 << i)))
      continue;
    else
    {	
      if(*RXBn_list.RXBnCON[i] & 0x80)
        return i;
    }
  }

  return -1;	// return invalid index buffer
}

/**************************************************
* Function name		: unsigned char can_load_buffer(unsigned char buffer, CAN_MESSAGE *message)
*	return			: <= 9: index of buffer
*					  -1 : not a free buffer
*	buffer			: valid tx buffer to be loaded. 
*					  If -1 the function search automatically a free buffer
*	message			: message to load into the buffer
*
* Created by		: Luca Lucci
* Date created		: 23/10/12
* Description		: If auto flag is passed, this function search for a free
*					  buffer and load it with data. The other buffers exist 
*					  only if can_init_prog_buffer_mode has been called with 1 
*					  with the second param. 
* Notes				: It should be always check if the returned buffer number is
*					  the passed one.
**************************************************/
unsigned char can_load_buffer(unsigned char buffer, CAN_MESSAGE *message)
{
  unsigned char buffer_index;
  unsigned char j; // variable for loop
  unsigned char *ptr;

  // check if I have to search for a free buffer or if the selected buffer is
  // waiting for other transmission. !
  if(buffer == -1 || (*TXBn_list.TXBnCON[buffer] & 0x08))
  {
    buffer_index = can_search_buffer_free();

	// if the buffer didn't be founded then return error
    if(buffer_index == -1)
	  return -1;
  }
  else	// load specified buffer
  {
    // If the buffer passed is a programmable buffer, check if it is a valid tx one
    if(buffer > 2 && !(BSEL0 & (0x01 << (buffer-1))))
      return -1;

    buffer_index = buffer;
  }

  //map TXBn register into TXB0xxx register
  //select the value to load in EWIN register
  switch(buffer_index)
  {
    case 0:
	case 1:
	case 2:
      ECANCONbits.EWIN = buffer_index + 3;
      break;
    case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	  ECANCONbits.EWIN = (buffer_index - 1);
	  ECANCONbits.EWIN4 = 1;
	  break;
    case -1:
      break;
	default:
	  return -1;
	  break;
  }

  //load data into the buffer
  // warning: all RXB0 register point to an tx register!!
  canid_to_regs((unsigned char *)&RXB0SIDH, message->id, message->exid_frame);

  RXB0DLC = message->data_length; // Load lenght of data

  // Load remote frame transmission flag
  RXB0DLCbits.RXRTR = message->rtr_frame;

  ptr = (unsigned char *)&RXB0D0;

  // Load data
  for(j = 0; j < message->data_length; j++)
  {
    ptr[j] = message->data[j];
  }

  // Restore default RXB0 mapping.
  ECANCONbits.EWIN = 0x10;

  return buffer_index;
}

/**************************************************
* Function name		: unsigned char can_write(unsigned char buffer)
* 	return			: 0 - failure: invalid buffer's index
*					: 1 - success
*	buffer_tx		: index of valid tx buffer
* Created by		: Luca Lucci
* Date created		: 24/10/12
* Description		: This function send the message stored in buffer arg. It
*					  should be passed as param the return value of  
*					  can_load_buffer() function
* Notes				: Calling this function don't initiate a message 
*					  transmission. Trasmission will start when the device
*					  detects that bus is available.
**************************************************/
unsigned char can_write(unsigned char buffer)
{
  // check if the select buffer is a tx one
  if(buffer > 2 && !(BSEL0 & (0x01 << (buffer-1))))
      return 0;

  // enable TXREQ to initiate transmission
  *TXBn_list.TXBnCON[buffer] |= 0x08;
  return 1;
}

/**************************************************
* Function name		: void can_read(CAN_MESSAGE *can_message, unsigned char buffer_index, unsigned char release_now)
*	return			: 0 = no data to read
*					  1 = can data loaded in rx buffer
*	can_message		: pointer to CAN_MESSAGE struct that will store all data
*	buffer_index	: buffer's number where data can be loaded
*   release_now     : 0 = don't release rx buffer
*					  1 = release rx buffer so it can receive the next message
* Created by		: Luca Lucci
* Date created		: 01/11/12
* Description		: This function read a message from rx buffer. It takes care
*					  to clear RXFUL and RXBnIF flags.
* Notes				: When release_now the function can_buffer_rx_release has 
*					  to be called, otherwise the buffer will not be released 
*                     and it will result alway to be read. Also the interrupt 
*					  flag will not be cleared, so user must disable interrupt 
*					  until can_buffer_rx_release is called
**************************************************/
void can_read(CAN_MESSAGE *can_message, unsigned char buffer_index, unsigned char release_now)
{
  unsigned char i;	// for loop
  unsigned char *ptr;	//to roll by data array

  // start with no error flags set
  can_message->rtr_frame = 0;
  can_message->exid_frame = 0;

  // Determine whether this was RTR or not
  if(*RXBn_list.RXBnCON[buffer_index] & 0x20)
    can_message->rtr_frame = 1;

  // Determine which filter was hit
  can_message->filter_hit = *RXBn_list.RXBnCON[buffer_index] & 0x1F;
  //Map the selected register in RXB0xxx
  ECANCONbits.EWIN = buffer_index;
  ECANCONbits.EWIN4 = 1;

  // Retreive message length
  can_message->data_length = RXB0DLC;
  

  // Retrieve EIDX bytes only if this is extended message
  if(RXB0SIDLbits.EXID)
  {
    can_message->exid_frame = 1;

    can_message->id = (RXB0SIDH << 20) | (RXB0SIDLbits.SID << 18) | 
					   (RXB0SIDLbits.EID << 16) | (RXB0EIDH << 8) | RXB0EIDL;
  }
  else
  {
    can_message->id = RXB0SIDH;
    can_message->id = (can_message->id << 3)|(RXB0SIDLbits.SID);
  }

  // Load Data

  ptr = (unsigned char *)&RXB0D0;

  for(i = 0; i < can_message->data_length; i++)
    can_message->data[i] = ptr[i];

  // Restore default RXB0 mapping.

//uart2_buffer_tx_load(can_message->data_length);
  ECANCONbits.EWIN = 0x10;

  if(release_now)
  {
    // Clear RXFUL flag, so it can be possible to receive another message
    *RXBn_list.RXBnCON[buffer_index] &= 0x7F;

    //Clear the received flag. This be done after clear RXFUL bit!
    PIR5bits.RXBnIF = 0;
  }
 
}

/**************************************************
* Function name		: void can_buffer_rx_release(unsigned char buffer_mask)
*	buffer_mask		: indicates which register has to be released
* Created by		: Luca Lucci
* Date created		: 23/03/13
* Description		: This function release rx buffers
* Notes				: This function should be called only if can_read has to be
*                     called with release_now clear. After this call the 
*					  interrupt can be restored
**************************************************/
void can_buffer_rx_release(unsigned char buffer_mask)
{
  unsigned char i;
  unsigned char buffer_index;

  for(i = 0; i < 8; i++)
  {
    // if the buffer is a tx one then update the buffer flag
    buffer_index = buffer_mask & (0x01 << i);
    
    if(buffer_index)
    {
      // Clear RXFUL flag, so it can be possible to receive another message
      *RXBn_list.RXBnCON[i] &= 0x7F;

      //Clear the received flag. This be done after clear RXFUL bit!
      PIR5bits.RXBnIF = 0;

      //update buffer_mask
      buffer_mask -= buffer_index;

      if(buffer_mask == 0)
        break;
    }
  }
}

/**************************************************
* Function name		: void canid_to_regs(unsigned char *buffer, unsigned long id, unsigned char exide)
*	buffer			: pointer to buffer xxxSIDH
*	id				: id to load into the buffer
*	exide			: flag for indicates extended ide or standard one
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function store can id in the registers form
* Notes				: -
**************************************************/
void canid_to_regs(unsigned char *buffer, unsigned long id, unsigned char exide)
{
  if(exide)	// extended id
  {
    *buffer = (id >> 21); // set SIDH
	buffer++; // pass to SIDL register

    *buffer = 8;			// set EXIDEN flag
    *buffer |= (id >> 13) & 0xE0;	// set SIDL
    *buffer |= (id >> 16) & 0x03;	// set 2 bit of EID
	buffer++;

    *buffer = (id >> 8) & 0xFF;	// set EIDH
    buffer++;

    *buffer = id & 0xFF;	// set EIDL
  }
  else
  {
    *buffer = id >> 3;	// set SIDH
    buffer++;

    *buffer = (id & 0x07) << 5;	// set SIDL and clear EXIDEN flag
  }
}

/**************************************************
* Function name		: void can_buffer_set_priority(unsigned char priority)
*   buffer_index	: buffer's number
*	priority		: 2 bit value that rapresent priority - '11' higher priority
*
* Created by		: Luca Lucci
* Date created		: 03/02/13
* Description		: Set the priority of the selected buffer
* Notes				: -
**************************************************/
void can_buffer_set_priority(unsigned char buffer_index, unsigned char priority)
{
  //I have to use TXBn_list due to the silicon errata
  *TXBn_list.TXBnCON[buffer_index] &= (0xFF & priority);
  *TXBn_list.TXBnCON[buffer_index] |= (0x00 | priority);
}
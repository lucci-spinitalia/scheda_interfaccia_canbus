/***************************************************
* FileName:        ricetrasmettitore.c
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
*   This module is an interface between mrf24j40 and can bus
****************************************************/

/** I N C L U D E S ********************************/
#include "ricetrasmettitore.h"
#include "spi_interface.h"
#include "mrf24j40.h"
#include <delays.h>

/** V A R I A B L E S  *****************************/
CAN_MESSAGE can_message; 	//old can message arrived
CAN_MESSAGE spi_message; 	//old can message arrived

unsigned char device_type;
unsigned char error_message[100];
unsigned char error_message_length;

/** D E F I N E ************************************/

// Sync param to setup the can baudrate for a 10 m cable
#if (FOSC_MHZ == 16)
  const unsigned char CAN_1000kbps[] = {1, 4, 1, 2, 0};
  const unsigned char CAN_500kbps[] = {4, 7, 4, 4, 0};
  const unsigned char CAN_250kbps[] = {4, 7, 4, 4, 1};
  const unsigned char CAN_125kbps[] = {4, 7, 4, 4, 3};
#elif (FOSC_MHZ == 64)
  const unsigned char CAN_1000kbps[] = {1, 4, 1, 2, 3};
  const unsigned char CAN_500kbps[] = {2, 3, 2, 2, 7};
  const unsigned char CAN_250kbps[] = {4, 3, 6, 6, 4};
  const unsigned char CAN_125kbps[] = {4, 1, 7, 7, 15};
#else
  #error "Frequenza micro non implementata"
#endif

/** P R I V A T E   P R O T O T Y P E   ************/
void can_spi_message_read_from_can(void);
void can_spi_message_read_from_spi(void);
unsigned char trasparent_can_state_machine(unsigned char current_state);
unsigned char trasparent_spi_state_machine(unsigned char current_state);

void InitMRF24J40(void);
unsigned char MFR24J40_message_send_short(unsigned char address, unsigned char message);
unsigned char MFR24J40_message_read_short(unsigned char address);
unsigned char MFR24J40_message_send_long(unsigned int address, unsigned char message);
unsigned char MFR24J40_message_read_long(unsigned int address);

/**************************************************
* Function name		: void can_spi_initialize(void)
*
* Created by		: Luca Lucci
* Date created		: 07/10/12
* Description		: module initialization
* Notes				: -
**************************************************/
void can_spi_initialize(void)
{
  unsigned char i; // for loop

  // initialize dip switch pins as inputs
  switch_sw1_tris = 1;
  switch_sw2_tris = 1;
  switch_sw3_tris = 1;
  switch_sw4_tris = 1;
  switch_sw5_tris = 1;
  switch_sw6_tris = 1;
  switch_sw7_tris = 1;
  switch_sw8_tris = 1;

  // define type of the device
  device_type = (switch_sw1_to_sw6) & 0x1F;

  // initialize spi
  spi_init();

  // initialize can bus
  // Init Can Bus Interface
  if(can_init(CAN_CONFIG, CAN_CONFIG_PROG, CAN_PROG_Bn_RX, (unsigned char *) CAN_1000kbps))
  {
    spi_message.id = 0x00;
	spi_message.rtr_frame = 0;
	spi_message.exid_frame = 0;

    // Initialize Masks to accept all message
    can_init_mask_id(0, 0b0000000000, 0);
    can_init_mask_id(1, 0b0000000000, 0);

    // Association buffer filters to acceptance mask. I want 1 mask for each
    // one half filter
    for(i = 0; i < 16; i++)
      can_init_filter_mask(i, i >> 3);

/*    // Set filter's id to address of device. If it is a pc interface accept all
    // Init filter 0 for broadcast message
    can_init_filter_id(0, uart_message.id & 0xF00, 0);
    // Init filter 1 for group message
    can_init_filter_id(1, uart_message.id & 0xFF0, 0);
    // Init other filter for normal message
    for(i = 2; i < 16; i++)
      can_init_filter_id(i, uart_message.id, 0);*/

    // Associate filter to buffer.
    can_init_filter_buffer(0,0);
    can_init_filter_buffer(1,1);
    can_init_filter_buffer(2,2);
    can_init_filter_buffer(3,2);
    can_init_filter_buffer(4,3);
    can_init_filter_buffer(5,3);
    can_init_filter_buffer(6,4);
    can_init_filter_buffer(7,4);
    can_init_filter_buffer(8,5);
    can_init_filter_buffer(9,5);
    can_init_filter_buffer(10,6);
    can_init_filter_buffer(11,6);
    can_init_filter_buffer(12,7);
    can_init_filter_buffer(13,7);
    can_init_filter_buffer(14,2);
    can_init_filter_buffer(15,3);

    // Enable/disable buffer filters
    can_init_filter_enable(RXF_ALL_ON);
  }

  // Init MRF24J40
  InitMRF24J40();
}

/**************************************************
* Function name		: void can_spi_loop(void)
*
* Created by		: Luca Lucci
* Date created		: 07/11/12
* Description		: function that has to be called in the main loop. It
*					  process uart and canbus message.
* Notes				: -
**************************************************/
void can_spi_loop(void)
{
  unsigned char return_value;

  can_spi_message_read_from_can();

  // Warning: this function can handle one message,so if the load ones update
  // the buffer with two or more message, the send function cannot flush them
  can_buffer_send();

  // Manage can error
  if(can_error_handle(error_message, &error_message_length))
	led_st3 = 1;

  can_spi_message_read_from_spi();

  spi_buffer_send();

}

/**************************************************
* Function name		: void can_spi_message_read_from_can(void)
*
* Created by		: Luca Lucci
* Date created		: 05/12/12
* Description		: This function read message from can and call the function
*					  to process it
* Notes				: When switch1 is off (sensor mode) only frame with the
*					  sensor id will be accepted. The follow structure allows to 
*				      implement common block that can be called once a cycle.
**************************************************/
void can_spi_message_read_from_can(void)
{
  static unsigned char can_message_read_state = 0;

  switch(can_message_read_state)
  {
	case 0:  //read message and process internal message
      //**************************************************
      // check can message received
      //**************************************************

      #ifndef CAN_INTERRUPT_RX
        //read the buffer that had received the new data
        can_buffer_rx_load();
      #endif

      //read the message from rx buffer
      if(!can_buffer_read(&can_message))
        break; //no message

	  can_message_read_state++;
	  break;

    default:  //manage device
	  switch(device_type)
      {
		case 0:	//this work only if all switches are opened
		  can_message_read_state = trasparent_can_state_machine(can_message_read_state);
		  break;

		default:
		  can_message_read_state = 0;
		  break;
	  }
	  break;
  }  
 
}

/**************************************************
* Function name		: void can_spi_message_read_from_spi(void)
*
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function read message from can and call the function
*					  to process it
* Notes				: 
**************************************************/
void can_spi_message_read_from_spi(void)
{
  static unsigned char spi_message_read_state = 0;

  switch(spi_message_read_state)
  {
	case 0:  //read message from spi
	  spi_message_read_state++;
	  break;

    default:  //manage device
      spi_message_read_state = trasparent_spi_state_machine(spi_message_read_state);
	  break;
  }  
 
}

/**************************************************
* Function name		: unsigned char trasparent_can_state_machine(unsigned char current_state)
*	return			: future state
*	current_state	: the current state of the machine
*
* Created by		: Luca Lucci
* Date created		: 28/01/13
* Description		: This function process the can message for redirect it
*					  directly to ricetrasmitter
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char trasparent_can_state_machine(unsigned char current_state)
{
  unsigned char return_value;	//store the value returned from macro

  switch(current_state)
  {
    case 1:	//send data over spi
	  if(spi_buffer_tx_seq_load((unsigned char *)&can_message.data, can_message.data_length))
      {
        // change led status when data has been received from device
        led_st2 = ~led_st2;

        current_state = 0;
      }

      break;

    default:
      current_state = 0;
      break;
  }

  return current_state;

}

/**************************************************
* Function name		: unsigned char trasparent_spi_state_machine(unsigned char current_state)
*	return			: future state
*	current_state	: the current state of the machine
*
* Created by		: Luca Lucci
* Date created		: 28/01/13
* Description		: This function process the can message for redirect it
*					  directly to ricetrasmitter
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char trasparent_spi_state_machine(unsigned char current_state)
{
  unsigned char data_ptr = 0; //take count of data read
  switch(current_state)
  {
    case 1:	//send data over spi
	  while(!spi_status.buffer_rx_empty && data_ptr < 8)
	  {
		spi_buffer_read(&spi_message.data[data_ptr]);
		data_ptr++;
 	  } 

	  if(data_ptr)
	  {
		spi_message.data_length = data_ptr;
		current_state++;
	  }
	  else
		break;

    case 2:
	  if(!can_buffer_tx_load(&spi_message))
		break;

	  led_st1 = ~led_st1;
	  current_state = 0;
	  break;

    default:
      current_state = 0;
      break;
  }

  return current_state;

}

void InitMRF24J40()
{
//1. SOFTRST (0x2A) = 0x07 – Perform a software Reset. The bits will be automatically cleared to ‘0’ by hardware.
  MFR24J40_message_send_short(SOFTRST, 0x07);

//2. PACON2 (0x18) = 0x98 – Initialize FIFOEN = 1 and TXONTS = 0x6.
  MFR24J40_message_send_short(PACON2, 0x98);

//3. TXSTBL (0x2E) = 0x95 – Initialize RFSTBL = 0x9.
  MFR24J40_message_send_short(TXSTBL, 0x95);

//4. RFCON0 (0x200) = 0x03 – Initialize RFOPT = 0x03.
  MFR24J40_message_send_long(RFCON0, 0x03);

//5. RFCON1 (0x201) = 0x01 – Initialize VCOOPT = 0x02.
  MFR24J40_message_send_long(RFCON1, 0x01);

//6. RFCON2 (0x202) = 0x80 – Enable PLL (PLLEN = 1).
  MFR24J40_message_send_long(RFCON2, 0x80);

//7. RFCON6 (0x206) = 0x90 – Initialize TXFIL = 1 and 20MRECVR = 1.
  MFR24J40_message_send_long(RFCON6, 0x90);

//8. RFCON7 (0x207) = 0x80 – Initialize SLPCLKSEL = 0x2 (100 kHz Internal oscillator).
  MFR24J40_message_send_long(RFCON7, 0x80);

//9. RFCON8 (0x208) = 0x10 – Initialize RFVCO = 1.
  MFR24J40_message_send_long(RFCON8, 0x10);

//10. SLPCON1 (0x220) = 0x21 – Initialize CLKOUTEN = 1 and SLPCLKDIV = 0x01.
  MFR24J40_message_send_long(SLPCON1, 0x21);

//11. BBREG2 (0x3A) = 0x80 – Set CCA mode to ED.
  MFR24J40_message_send_short(BBREG2, 0x80);

//12. CCAEDTH = 0x60 – Set CCA ED threshold.
  MFR24J40_message_send_short(CCAEDTH, 0x60);

//13. BBREG6 (0x3E) = 0x40 – Set appended RSSI value to RXFIFO.
  MFR24J40_message_send_short(BBREG6, 0x40);

//14. Disable interrupts – See Section 3.3 “Interrupts”.
//15. Set channel – See Section 3.4 “Channel Selection”.
//  MFR24J40_message_send_long(RFCON0, 0x03);

//16. Set transmitter power - See “REGISTER 2-62: RF CONTROL 3 REGISTER (ADDRESS: 0x203)”.
  MFR24J40_message_send_long(RFCON3, 0x00);

// 17. RFCTL (0x36) = 0x04 – Reset RF state machine.
  MFR24J40_message_send_short(RFCTL, 0x04);

//18. RFCTL (0x36) = 0x00.
  MFR24J40_message_send_short(RFCTL, 0x00);

  Delay100TCYx(35);
}

/**************************************************
* Function name		: unsigned char MFR24J40_send_message_short(unsigned char address, unsigned char message)
*	address	  		: 6 bit destination address
*	message			: 1 byte of message
*
* Created by		: Luca Lucci
* Date created		: 19/02/13
* Description		: This function send a short address message to the 
*					  MRF24J40 module
* Notes				:
**************************************************/
unsigned char MFR24J40_message_send_short(unsigned char address, unsigned char message)
{
  unsigned char address_write;

  address_write = ((address << 1) & 0x7E) + 1;

  chip_select_enable();

  SSPBUF = address_write;

  do
  {
	SSPBUF = message;
  }while(SSPCON1bits.WCOL);

  chip_select_disable();

  return 1;
}

/**************************************************
* Function name		: unsigned char MFR24J40_message_read_short(unsigned char address)
*	address	  		: 6 bit destination address
*
* Created by		: Luca Lucci
* Date created		: 19/02/13
* Description		: This function send a short address message to the 
*					  MRF24J40 module
* Notes				:
**************************************************/
unsigned char MFR24J40_message_read_short(unsigned char address)
{
  unsigned char address_read;

  address_read = ((address << 1) & 0x7E);

  chip_select_enable();

  SSPBUF = address_read;

  while(!SSPSTATbits.BF);

  return SSPBUF;
}


/**************************************************
* Function name		: unsigned char MFR24J40_send_message_long(unsigned int address, unsigned char message)
*	address	  		: 10 bit destination address
*	message			: 1 byte of message
*
* Created by		: Luca Lucci
* Date created		: 19/02/13
* Description		: This function send a short address message to the 
*					  MRF24J40 module
* Notes				:
**************************************************/
unsigned char MFR24J40_message_send_long(unsigned int address, unsigned char message)
{
  unsigned char address_write;

  address_write = ((address << 1) & 0xFFE) | 0x801;

  chip_select_enable();

  // send address_write MSB
  SSPBUF = (address_write) >> 4;

  // send address_write LSB
  do
  {
    SSPBUF = (address_write) << 4;
  }while(SSPCON1bits.WCOL);

  // send 4 MSB out
  do
  {
	SSPBUF = message;
  }while(SSPCON1bits.WCOL);

  chip_select_disable();

  return 1;
}

/**************************************************
* Function name		: unsigned char MFR24J40_message_read_long(unsigned int address)
*	address	  		: 10 bit destination address
*
* Created by		: Luca Lucci
* Date created		: 19/02/13
* Description		: This function send a short address message to the 
*					  MRF24J40 module
* Notes				:
**************************************************/
unsigned char MFR24J40_message_read_long(unsigned int address)
{
  unsigned char address_read;

  address_read = ((address << 1) & 0xFFE) | 0x800;

  chip_select_enable();

  // send address_write MSB
  SSPBUF = (address_read) >> 4;

  // send address_write LSB
  do
  {
    SSPBUF = (address_read) << 4;
  }while(SSPCON1bits.WCOL);

  SSPBUF = address_read;

  while(!SSPSTATbits.BF);

  return SSPBUF;
}
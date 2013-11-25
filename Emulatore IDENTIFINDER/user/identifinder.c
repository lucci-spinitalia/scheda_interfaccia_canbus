/***************************************************
* FileName:        can_to_rs232_converter.c
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
****************************************************/

/** I N C L U D E S ********************************/
#include "identifinder.h"
//#include <string.h>
#include <stdio.h>

/** D E F I N E ************************************/
#define BUFFER_SIZE 100

/** V A R I A B L E S  *****************************/
CAN_MESSAGE can_message; 	//old can message arrived
CAN_MESSAGE uart_message;	//old uart message arrived

// Variable for uart configuration. These have been decleared to abstract
// function to serial's number choose.
unsigned char uart_interrupt_tx;
unsigned char uart_interrupt_rx;
unsigned long uart_baud_rate;
struct uart_status *uartx_status;
unsigned char uart_number;
unsigned char debug_interrupt_tx;

unsigned char buffer[BUFFER_SIZE];
unsigned char cmd_cs000[] = "!cs000";
unsigned char cmd_ser[] = "ser";
unsigned char cmd_sah[] = "sah";
unsigned char cmd_cs[] = "?cs";
unsigned char cmd_sas[] = "sas";

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
void can_uart_message_read_from_can(void);
void can_uart_proc_int_message(unsigned char message);
unsigned char identifinder_uart_state_machine(unsigned char current_state);
unsigned char identifinder_can_state_machine(unsigned char current_state);

/**************************************************
* Function name		: void can_uart_initialize(void)
*
* Created by		: Luca Lucci
* Date created		: 07/10/12
* Description		: module initialization
* Notes				: -
**************************************************/
void can_uart_initialize(void)
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

  // initialize uart
  uart_number = 1;

  uartx_status = &uart1_status;
  uart_baud_rate = UART1_BAUD_RATE;
  uart_interrupt_tx = UART1_INTERRUPT_TX;
  uart_interrupt_rx = UART1_INTERRUPT_RX;

  debug_interrupt_tx = UART2_INTERRUPT_TX;

  uart1_tx_tris = OUTPUT_PIN;
  uart1_rx_tris = INPUT_PIN;
  uart2_tx_tris = OUTPUT_PIN;
  uart2_rx_tris = INPUT_PIN;

  MAX3160_ENABLE();
  MAX3160_ENABLE_RS232();
  uart_init(RS232);
  uart_open(38400);
	  
  //init debug interface
  MAX3160_ENABLE_DEBUG();
  MAX3160_ENABLE_RS232_DEBUG();
  debug_init(RS232);
  debug_open(38400);

  // initialize can bus
  // Init Can Bus Interface
  if(can_init(CAN_CONFIG, CAN_CONFIG_PROG, CAN_PROG_Bn_RX, (unsigned char *) CAN_1000kbps))
  {
    uart_message.id = SENSOR_RWA_IDENTIFINDER_ADDR;

    // Initialize Masks to accept all message
    can_init_mask_id(0, 0b0000000000, 0);
    can_init_mask_id(1, 0b0000000000, 0);

    uart_message.rtr_frame = 0;
    uart_message.exid_frame = 0;

    // Association buffer filters to acceptance mask. I want 1 mask for each
    // one half filter
    for(i = 0; i < 16; i++)
      can_init_filter_mask(i, i >> 3);

    // Set filter's id to address of device. If it is a pc interface accept all
    // Init filter 0 for broadcast message
    can_init_filter_id(0, uart_message.id & 0xF00, 0);
    // Init filter 1 for group message
    can_init_filter_id(1, uart_message.id & 0xFF0, 0);
    // Init other filter for normal message
    for(i = 2; i < 16; i++)
      can_init_filter_id(i, uart_message.id, 0);

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

  can_bus_enable_terminator;
}

/**************************************************
* Function name		: void can_uart_loop(void)
*
* Created by		: Luca Lucci
* Date created		: 07/11/12
* Description		: function that has to be called in the main loop. It
*					  process uart and canbus message.
* Notes				: -
**************************************************/
void can_uart_loop(void)
{
  unsigned char return_value;

  can_uart_message_read_from_can();

  // Warning: this function can handle one message,so if the load ones update
  // the buffer with two or more message, the send function cannot flush them
  if(!can_buffer_send())
	led_st3 = 1;

  // if interrupt is disabled then polling send function for uart
  if(!uart_interrupt_tx)
    uart_buffer_send();

  // Manage can error
  if(can_error_handle(error_message, &error_message_length))
  {
	led_st3 = 1;
    debug_buffer_tx_seq_load(return_value, error_message, error_message_length);
  }

  // Manage uart error
  uart_error_handle(return_value, error_message, &error_message_length);
  if(return_value)
  {
    led_st3 = 1;
    debug_buffer_tx_seq_load(return_value, error_message, error_message_length);
  }

  // if interrupt is disabled then polling send function for uart
  if(!debug_interrupt_tx)
    debug_buffer_send();
}

/**************************************************
* Function name		: void can_uart_message_read_from_can(void)
*
* Created by		: Luca Lucci
* Date created		: 05/12/12
* Description		: This function read message from can and call the function
*					  to process it
* Notes				: When switch1 is off (sensor mode) only frame with the
*					  sensor id will be accepted. The follow structure allows to 
*				      implement common block that can be called once a cycle.
**************************************************/
void can_uart_message_read_from_can(void)
{
  static unsigned char can_message_read_state = 0;
  static unsigned char old_state = 0;	//remove

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

    case 1:  //manage device
	 	  uart1_buffer_tx_load(0xaa);
      //can_message_read_state = identifinder_can_state_machine(can_message_read_state);
	  
	  can_message_read_state = 0;
	  break;
  }  
 
}

/**************************************************
* Function name		: unsigned char identifinder_can_state_machine(unsigned char current_state)
*	return			: future state
*	current_state	: the current state of the machine
*
* Created by		: Luca Lucci
* Date created		: 28/01/13
* Description		: This function process the can message for the "sass2300"
*				      device.
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char identifinder_can_state_machine(unsigned char current_state)
{
  static unsigned char ptr = 0;
  unsigned char return_value;	//store the value returned from macro
  unsigned char return_value_memcp;
  unsigned char i;

  switch(current_state)
  {
    case 1:	//send data over uart
      //flush the buffer3		
	  if(BUFFER_SIZE < can_message.data_length + ptr)
	  {
		led_st3 = 1;
        //current_state++;
		ptr = 0;
	    break;
	  }
	  
      for(i = 0; i < can_message.data_length; i++)
	  {
		buffer[ptr] = can_message.data[i];
		ptr++;
	  }

      //uart_buffer_tx_load(return_value, buffer[data_ptr - 1]);
	  
      if(buffer[ptr - 1] == 0x0D)
	  {
	    current_state++;
	    break;
	  } 

	  current_state = 0;
      break;

    case 2:	//find command
	  return_value_memcp =memcmp(buffer, cmd_ser, sizeof(cmd_ser)-1);
	  uart_buffer_tx_load(return_value, return_value_memcp);

	  if(return_value_memcp == 0)
	  {		
		led_st2 = ~led_st2;

		uart_message.data_length = sprintf((unsigned char *)uart_message.data,"ser 3555");
        uart_buffer_tx_seq_load(return_value, uart_message.data, uart_message.data_length);

		if(!can_buffer_tx_load(&uart_message))
		  break;

		/*uart_message.data_length = sprintf((unsigned char *)uart_message.data,"-12\r\n");
		uart_buffer_tx_seq_load(return_value, uart_message.data, uart_message.data_length);
		
        if(!can_buffer_tx_load(&uart_message))
		{
		  led_st3 = 1;
		  break;
  		} 
		
		uart_message.data_length = sprintf((unsigned char *)uart_message.data," OK:  ");
		uart_buffer_tx_seq_load(return_value, uart_message.data, uart_message.data_length);
		
        if(!can_buffer_tx_load(&uart_message))
	    {
		  led_st3 = 1;
		  break;
		}

		//uart_message.data_length =  sprintf((unsigned char *)buffer, "Sent ser command /r/n");
		//uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, uart_message.data_length);
      }   
      /*else if(!memcmp(buffer, cmd_cs000, sizeof(cmd_cs000)-1))
	  {
		led_st2 = ~led_st2;
		sprintf((unsigned char *)uart_message.data,"  000sc!");
		uart_message.data[1] = 0x0D;
		uart_message.data[0] = 0x0A;

		uart_message.data_length = (sizeof("000sc!") - 1) + 2;
		
		//uart_buffer_tx_seq_load(return_value, uart_message.data,uart_message.data_length);
		if(!can_buffer_tx_load(&uart_message))
		  break;
		
		sprintf((unsigned char *)uart_message.data,"  :KO ");
		uart_message.data_length = (sizeof("  :KO ") - 1);

		if(!can_buffer_tx_load(&uart_message))
		  break;

		sprintf((unsigned char *)buffer, "  000sc! tneS");
		
        uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, sizeof("  000sc! tneS")-1);*/
	
	  }
      /*else if(!memcmp(buffer, cmd_sah, sizeof(cmd_sah)-1))
	  {
		led_st2 = ~led_st2;
        uart_message.data[7] = 0x73;
		uart_message.data[6] = 0x61;
		uart_message.data[5] = 0x68;
		uart_message.data[4] = 0x00;
		uart_message.data[3] = 0x00;
		uart_message.data[2] = 0x00;
		uart_message.data[1] = 0x00;
		uart_message.data[0] = 0x00;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

        uart_message.data[7] = 0x01;
		uart_message.data[6] = 0x00;
		uart_message.data[5] = 0x10;
		uart_message.data[4] = 0x30;
		uart_message.data[3] = 0x01;
		uart_message.data[2] = 0x13;
		uart_message.data[1] = 0x00;
		uart_message.data[0] = 0x15;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

        uart_message.data[7] = 0x25;
		uart_message.data[6] = 0x37;
		uart_message.data[5] = 0x00;
		uart_message.data[4] = 0x30;
		uart_message.data[3] = 0x01;
		uart_message.data[2] = 0x13;
		uart_message.data[1] = 0x00;
		uart_message.data[0] = 0x15;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

        uart_message.data[7] = 0x26;
		uart_message.data[6] = 0x07;
		uart_message.data[5] = 0x00;
		uart_message.data[4] = 0x00;
		uart_message.data[3] = 0x00;
		uart_message.data[2] = 0x00;
		uart_message.data[1] = 0x00;
		uart_message.data[0] = 0x00;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

        uart_message.data[7] = 0x00;
		uart_message.data[6] = 0x00;
		uart_message.data[5] = 0x00;
		uart_message.data[4] = 0x00;
		uart_message.data[3] = 0x00;
		uart_message.data[2] = 0x00;
		uart_message.data[1] = 0x00;
		uart_message.data[0] = 0x00;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

        uart_message.data[7] = 0x00;
		uart_message.data[6] = 0x61;
		uart_message.data[5] = 0x6B;
		uart_message.data[4] = 0x74;
		uart_message.data[3] = 0x53;
		uart_message.data[2] = 0x70;
		uart_message.data[1] = 0x65;
		uart_message.data[0] = 0x63;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

        uart_message.data[7] = 0x46;
		uart_message.data[6] = 0x2E;
		uart_message.data[5] = 0x73;
		uart_message.data[4] = 0x70;
		uart_message.data[3] = 0x63;
		uart_message.data[2] = 0x00;
		uart_message.data[1] = 0x00;
		uart_message.data[0] = 0x00;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

        uart_message.data[7] = 0x00;
		uart_message.data[6] = 0x00;
		uart_message.data[5] = 0x00;
		uart_message.data[4] = 0x00;
		uart_message.data[3] = 0x00;
		uart_message.data[2] = 0x00;
		uart_message.data[1] = 0x00;
		uart_message.data[0] = 0x00;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

        uart_message.data[7] = 0x00;
		uart_message.data[6] = 0x00;
		uart_message.data[5] = 0x00;
		uart_message.data[4] = 0x0D;
		uart_message.data[3] = 0x0A;
		uart_message.data[2] = 0x20;
		uart_message.data[1] = 0x4F;
		uart_message.data[0] = 0x4B;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

	    uart_message.data[2] = 0x3A;
        uart_message.data[1] = 0x20;
		uart_message.data[0] = 0x20;

		uart_message.data_length = 3;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

		sprintf((unsigned char *)buffer, "  has tneS");
        uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, sizeof("  has tneS")-1);
	  }
      else if(!memcmp(buffer, cmd_cs, sizeof(cmd_cs)-1))
      {
		led_st2 = ~led_st2;
		uart_message.data[6] = 0x3F;
		uart_message.data[5] = 0x63;
		uart_message.data[4] = 0x73;
		uart_message.data[3] = 0x44;
		uart_message.data[2] = 0x31;
		uart_message.data[1] = 0x0D;
		uart_message.data[0] = 0x0A;

		uart_message.data_length = 7;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		sprintf((unsigned char *)uart_message.data,"  :KO ");
		uart_message.data_length = (sizeof("  :KO ") - 1);

		if(!can_buffer_tx_load(&uart_message))
		  break;

		sprintf((unsigned char *)buffer, "  sc tneS");
		uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, sizeof("  sc tneS")-1);
      }
      else if(!memcmp(buffer, cmd_sas, sizeof(cmd_sas)-1))
      {
		led_st2 = ~led_st2;
		uart_message.data[7] = 0x73;
		uart_message.data[6] = 0x61;
		uart_message.data[5] = 0x73;
		uart_message.data[4] = 0x7F;
		uart_message.data[3] = 0x7F;
		uart_message.data[2] = 0x7F;
		uart_message.data[1] = 0x7F;
		uart_message.data[0] = 0x7F;

		uart_message.data_length = 8;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

		can_buffer_send();

		for(i = 0; i < 127; i++)
        {
		  uart_message.data[7] = 0x7F;
		  uart_message.data[6] = 0x7F;
		  uart_message.data[5] = 0x7F;
		  uart_message.data[4] = 0x7F;
		  uart_message.data[3] = 0x7F;
		  uart_message.data[2] = 0x7F;
		  uart_message.data[1] = 0x7F;
		  uart_message.data[0] = 0x7F;

		  uart_message.data_length = 8;
		
		  if(!can_buffer_tx_load(&uart_message))
		    break;

		  can_buffer_send();
		}

		uart_message.data[4] = 0x7F;
		uart_message.data[3] = 0x7F;
		uart_message.data[2] = 0x7F;
		uart_message.data[1] = 0x0D;
		uart_message.data[0] = 0x0A;

		uart_message.data_length = 5;
		
		if(!can_buffer_tx_load(&uart_message))
		  break;

	    can_buffer_send();

		sprintf((unsigned char *)uart_message.data,"  :KO ");
		uart_message.data_length = (sizeof("  :KO ") - 1);

		if(!can_buffer_tx_load(&uart_message))
		  break;

		sprintf((unsigned char *)buffer, "  sas tneS");
		uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, sizeof("  sas tneS")-1);
      }*/
	  else
	  {
		//uart_buffer_tx_seq_load(return_value, (unsigned char *)cmd_ser, sizeof(cmd_ser)-1);
		//print the unknown command on the screen
		//uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, sizeof(cmd_ser)-1);
	  }

	  ptr = 0;

	  current_state = 0;
	  break;

    default:
      current_state = 0;
      break;
  }

  return current_state;

}


/**************************************************
* Function name		: unsigned char identifinder_uart_state_machine(unsigned char current_state)
*	return			: future state
*	current_state	: the current state of the machine
*
* Created by		: Luca Lucci
* Date created		: 28/01/13
* Description		: This function process the uart message for the "sass2300"
*				      device.
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char identifinder_uart_state_machine(unsigned char current_state)
{
  unsigned char return_value;	//store the value returned from macro
  unsigned char data_ptr = 0;	//take count of data read

  switch(current_state)
  {
    case 1:	//send data over can
      while(!uartx_status->buffer_rx_empty && data_ptr < 8)
	  {
        uart_buffer_read(return_value, &uart_message.data[data_ptr]); // read data from uart
        data_ptr++;
	  }
	
	  if(data_ptr)
	  {
        //update length value
        uart_message.data_length = data_ptr;

	    current_state++;
	  }
	  else
		break;

	  //don't put a break here!!!!

     case 2:
      // load can tx buffer. If there's no buffer left, it will be assert queue 
      // flag

      if(!can_buffer_tx_load(&uart_message))
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


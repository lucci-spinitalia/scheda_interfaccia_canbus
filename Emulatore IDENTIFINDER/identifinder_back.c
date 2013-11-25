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
* The purpouse of this module is to emulate the identifinder device. The 
* command have been sniffed from rs232 communication between identifinder and
* winTMCA software
*
****************************************************/

/** I N C L U D E S ********************************/
#include "identifinder.h"
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 100

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

/** V A R I A B L E S  *****************************/
CAN_MESSAGE can_message; 	//old can message arrived
CAN_MESSAGE uart_message;	//old uart message arrived

unsigned char uart_number;	//serial driver selection (1 or 2)

// Variable for uart configuration. These have been decleared to abstract
// function to serial's number choose.
unsigned char uart_interrupt_tx;
unsigned char uart_interrupt_rx;
unsigned long uart_baud_rate;
struct uart_status *uartx_status;

unsigned char buffer[BUFFER_SIZE];	//buffer where store character for identifinder
unsigned char cmd_cs000[] = "!cs000";
unsigned char cmd_ser[] = "ser";
unsigned char cmd_sah[] = "sah";
unsigned char cmd_cs[] = "?cs";
unsigned char cmd_sas[] = "sas";

/** P R I V A T E   P R O T O T Y P E   ************/
void identifinder_message_read_from_uart(void);
void identifinder_message_read_from_can(void);

unsigned char identifinder_uart_state_machine(unsigned char current_state);
unsigned char identifinder_can_state_machine(unsigned char current_state);



/**************************************************
* Function name		: void identifinder_initialize(void)
*
* Created by		: Luca Lucci
* Date created		: 07/10/12
* Description		: module initialization
* Notes				: -
**************************************************/
void identifinder_initialize(void)
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

  // Driver select
  uart_number = switch_sw6 + 1;

  // Select uart structure
  if(uart_number ==1)
  {
	uartx_status = &uart1_status;
    uart_baud_rate = UART1_BAUD_RATE;
    uart_interrupt_tx = UART1_INTERRUPT_TX;
    uart_interrupt_rx = UART1_INTERRUPT_RX;
  }
  else
  {
    uartx_status = &uart2_status;
    uart_baud_rate = UART2_BAUD_RATE;
    uart_interrupt_tx = UART2_INTERRUPT_TX;
    uart_interrupt_rx = UART2_INTERRUPT_RX;
  }

  uart1_tx_tris = OUTPUT_PIN;
  uart1_rx_tris = INPUT_PIN;
  uart2_tx_tris = OUTPUT_PIN;
  uart2_rx_tris = INPUT_PIN;

  // Choose the uart type
  MAX3160_ENABLE();
  MAX3160_ENABLE_RS232();
  uart_init(RS232);
  uart_open(38400);

  // initialize can bus

  // Init Can Bus Interface
  if(can_init(CAN_CONFIG, CAN_CONFIG_PROG, CAN_PROG_Bn_RX, CAN_1000kbps))
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
}

/**************************************************
* Function name		: void identifinder_loop(void)
*
* Created by		: Luca Lucci
* Date created		: 07/11/12
* Description		: function that has to be called in the main loop. It
*					  process uart and canbus message.
* Notes				: -
**************************************************/
void identifinder_loop(void)
{
  //identifinder_message_read_from_uart();

  can_buffer_send();

  identifinder_message_read_from_can();

  if(!uart_interrupt_tx)
	uart_buffer_send();
}

/**************************************************
* Function name		: void identifinder_message_read_from_uart(void)
*
* Created by		: Luca Lucci
* Date created		: 05/12/12
* Description		: This function read message from uart and call the function
*					  to process it
* Notes				: When switch1 is on (pc interface mode) only valid frame
*					  will be passed through.
*					  Warning!! When in "pc interface" mode, the main loop will
*					  be slowed due to the additional information send through.
*					  This can bring to a can buffer rx overflow. In this 
*					  situation the max buffer size value in can_interface.h have to be changed
*					  have to be changed
**************************************************/
void identifinder_message_read_from_uart(void)
{
  static unsigned char uart_message_read_state = 0;

  unsigned char result;	//result from macro

  if(uart_interrupt_rx == 0)
    uart_buffer_rx_load(); // without interrupt I have to poll usart

  switch(uart_message_read_state)
  {
    case 0:  //common task
	  uart_message_read_state++;
	  break;

	default:  //manage device
	  uart_message_read_state = identifinder_uart_state_machine(uart_message_read_state);
	  break;
  }

}

/**************************************************
* Function name		: void identifinder_message_read_from_can(void)
*
* Created by		: Luca Lucci
* Date created		: 05/12/12
* Description		: This function read message from can and call the function
*					  to process it
* Notes				: When switch1 is off (sensor mode) only frame with the
*					  sensor id will be accepted. The follow structure allows to 
*				      implement common block that can be called once a cycle.
**************************************************/
void identifinder_message_read_from_can(void)
{
  static unsigned char can_message_read_state = 0;
  unsigned char return_value;	//store the value returned from macro

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
	  /*else
	  {
		//uart_buffer_tx_load(return_value, can_message.data[0]);
	  }*/

	  //can_message_read_state++;
	  break;

    default:  //manage device
	  can_message_read_state = identifinder_can_state_machine(can_message_read_state);
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
* Description		: This function process the can message for the "pc 
*					  interface" device.
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char identifinder_can_state_machine(unsigned char current_state)
{
  unsigned char i;
  unsigned char return_value;	//store the value returned from macro
  static unsigned char data_ptr = 0;

  // In this case I would like to send even small portion of data from a large
  // data frame. I need this state machine due to the uart buffer overflow 
  // event: every time I can't send message to uart I would like to try to send
  // the same byte.
  switch(current_state)
  {
    case 1:	//store data until CR (0x0D)
      //flush the buffer
	  if((BUFFER_SIZE - data_ptr) < can_message.data_length)
	  {
        current_state++;
	    break;
	  }
	  
      for(i = 0; i < can_message.data_length; i++)
	  {
		buffer[data_ptr] = can_message.data[i];
		data_ptr++;
	  }

      //uart_buffer_tx_seq_load(return_value, (unsigned char *)&buffer[data_ptr], can_message.data_length);
      uart_buffer_tx_load(return_value, buffer[data_ptr - 1]);
	  
     if(buffer[data_ptr - 1] == 0x0D)
	  {
	    current_state++;
		break;
	  }

	  current_state = 0;
      break;

	case 2:	//find command
	  if(!memcmp(buffer, cmd_ser, sizeof(cmd_ser)-1))
	  {		
		led_st2 = ~led_st2;
		sprintf((unsigned char *)uart_message.data,"ser 3555");
		uart_message.data_length = 8;
		
        uart_buffer_tx_seq_load(return_value, uart_message.data, uart_message.data_length);

		if(!can_buffer_tx_load(&uart_message))
		  break;

		uart_message.data_length = sprintf((unsigned char *)uart_message.data,"-12\r\n");

		uart_buffer_tx_seq_load(return_value, uart_message.data, uart_message.data_length - 1);
		if(!can_buffer_tx_load(&uart_message))
		{
		  break;
	     }   
		
		sprintf((unsigned char *)uart_message.data,"  :KO ");
		uart_message.data_length = (sizeof("  :KO ") - 1);

		uart_buffer_tx_seq_load(return_value, uart_message.data,uart_message.data_length);
		if(!can_buffer_tx_load(&uart_message))
		  break;

		sprintf((unsigned char *)buffer, "  res tneS");
		uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, sizeof("  res tneS")-1);
      }   
      else if(!memcmp(buffer, cmd_cs000, sizeof(cmd_cs000)-1))
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
		
        uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, sizeof("  000sc! tneS")-1);
	
	  }
      else if(!memcmp(buffer, cmd_sah, sizeof(cmd_sah)-1))
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
      }
	  else
	  {
		//print the unknown command on the screen
		uart_buffer_tx_seq_load(return_value, (unsigned char *)buffer, data_ptr);
	  }

	  data_ptr = 0;

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
* Description		: This function process the uart message for the "pc 
*					  interface" device.
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char identifinder_uart_state_machine(unsigned char current_state)
{
  unsigned char return_value;	//store the value returned from macro
  unsigned char data_ptr = 0;	// for fill the data

  static unsigned char uart_data_count = 0;  //to count bytes received form usart

  // exit if there's nothing to read
  if(uartx_status->buffer_rx_empty)
    return current_state;

  //**************************************************
  // check uart message received
  //**************************************************
  // Get data until buffer is empty or reach the maximum data that can be sent
  // in one can frame. Due to can frame format, the state machine store as much
  // data as possible from uart to fill all data portion
  switch(current_state)
  {
    case 1:	//no message to send; waiting for header
      uart_buffer_read(&uart_message.data[data_ptr]); // read data from uart

      if(uart_message.data[data_ptr] == HEADER)
	    current_state++;
		
	  // stop if there's nothing to read
	  if(uartx_status->buffer_rx_empty) 
	    break;

	case 2:	//read first index byte and send over can bus
	  uart_buffer_read(&uart_message.data[data_ptr]); // read data from uart

	  uart_message.id = ((unsigned long)uart_message.data[data_ptr] & 0x0007) << 8;
	  current_state++;

	  // stop if there's nothing to read
	  if(uartx_status->buffer_rx_empty) 
	    break;

	case 3:	//read second index byte and send over can bus	
	  uart_buffer_read(&uart_message.data[data_ptr]); // read data from uart

	  uart_message.id |= uart_message.data[data_ptr];
	  current_state++;

	  // stop if there's nothing to read
	  if(uartx_status->buffer_rx_empty) 
	    break;

	case 4:	//read length byte
  	  uart_buffer_read(&uart_message.data[data_ptr]); // read data from uart

	  uart_data_count = uart_message.data[data_ptr];

	  // If data length is zero then it is an internal message.
	  if(uart_data_count == 0)
	    current_state = 7;
	  else
	    current_state++;

	  // stop if there's nothing to read
	  if(uartx_status->buffer_rx_empty) 
	    break;

	case 5:	//send data over can bus
      while(!uartx_status->buffer_rx_empty && data_ptr < 8)
	  {
	    uart_buffer_read(&uart_message.data[data_ptr]); // read data from uart

	    data_ptr++;
	    uart_data_count--;

	    if(!uart_data_count)
	    {
	      // data sent; return to initial state
	      current_state = 0;

	      break;
	    }
      }  
	  
	  if(data_ptr)
        current_state++;

	//in this point don't take a break cause I want to send the message as soon
	//as possible and, if it fails, restart from here
         
    case 6:
      //update length value
      uart_message.data_length = data_ptr;

      // load can tx buffer. If there's no buffer left, it will be assert queue 
      // flag
      if(!can_buffer_tx_load(&uart_message))
        break;

      // send uart message on can bus
      if(!can_status.buffer_tx_empty)
	  {
        // change led status when data has been received from device
        led_st1 = ~led_st1;
        can_buffer_send();
		current_state = 0;
	  }
   
      break;

	case 7:  //process internal message
	  //can_uart_proc_int_message(uart_message.data[data_ptr]);

	  // return to initial state
	  current_state = 0;
	  break;

	default:	
	  current_state = 0;
	  break;
  	
  }

  return current_state;
}
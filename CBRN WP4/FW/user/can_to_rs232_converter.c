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
* The purpouse of this module is interface can bus to rs232. The most important
* function is uart_to_canbus_process that must be called in the main loop. 
* This module send message over rs232 without any added data. If SW1 of dip
* switch is on, then the module add 1 byte header and 1 byte index and 1 byte 
* data length before the payload.
*
* DIP SWITCH SW1:
*   0: only payload
*	1: send header (1 byte) - index (4 byte) - payload
*
* DIP SWITCH SW2-SW5: DEVICE INDEX
*
* DIP SWITCH SW6:
*   0: serial driver 1
*   1: serial driver 2
*
* DIP SWITCH SW7-SW8
*	00: SWITCH OFF SERIAL INTERFACE
*	01: RS232
*	10: RS485 - not implemented yet
*	11: RS422 - not implemented yet
*
* LED:
*   ST1: data received from device
*   ST2: data send to device
*
* To send a message to the board in pc interface mode the uart frame must be:
*	|HEADER|ADDR1|ADDR2|LENGTH|DATA...|
* Where:
*	HEADER is a predefined character
*	ADDR1 is the most important byte of the destination address
*   ADDR2 is the least important byte of the destination address
*	LENGTH is the number of data byte
*	DATA is the payload
*
* To send an internal message to the board in pc interface mode one have to be
* set LENGTH to 0. The first byte of data will be the command to the board.
*
* To send an internal message to the board in sensor mode one have to send 1
* byte of data at broadcast or group address
*
* The command codes available are:
*	0x00 -> reset the micro
*	0x01 -> set rs-232 baudrate at 9600
*	0x02 -> set rs-232 baudrate at 19200
*	0x03 -> set rs-232 baudrate at 38400
*	0x04 -> set rs-232 baudrate at 57600
*	0x05 -> set rs-232 baudrate at 115200
*	0x06 -> set can bus baudrate at 125kbps
*	0x07 -> set can bus baudrate at 250kbps
*	0x08 -> set can bus baudrate at 500kbps
*	0x09 -> set can bus baudrate at 1000kbps
*	0x0A -> enable terminator resistor
*	0x0B -> disable terminator resistor
****************************************************/

/** I N C L U D E S ********************************/
#include "can_to_rs232_converter.h"
#include "cbrn.h"

/** V A R I A B L E S  *****************************/
CAN_MESSAGE can_message; 	//old can message arrived
CAN_MESSAGE uart_message;	//old uart message arrived

//can message that can't be sent on canbus due to the buffer full
unsigned char uart_message_queue;
 
unsigned char uart_number;	//serial driver selection (1 or 2)
unsigned char uart_enabled;	//hardware flag to open selected uart

//state machine for uart module when it is in pc interface configuration
//	0: no message to transmitt over can bus
//  1: header received, waiting for firs index byte
//  2: first index byte received; waiting for second index byte
//  3: index bytes received; waiting for length byte
//  4: length byte received; send data over can_bus
unsigned char uart_pc_interface_state;
//state machine for uart module when it is in pc interface configuration
//	0: send header
//  1: send first address byte
//  2: send second address byte
//  3: send length byte
//  4: send data
unsigned char can_pc_interface_state;
unsigned char uart_data_count;	//to count bytes received form usart

// Variable for uart configuration. These have been decleared to abstract
// function to serial's number choose.
unsigned char uart_interrupt_tx;
unsigned char uart_interrupt_rx;
unsigned long uart_baud_rate;
struct uart_status *uartx_status;

unsigned char *temp;		// for test only. remove
unsigned char data[] = "data: "; // created for test purpouse only. remove it
unsigned char newline[] = "\n";    // created for test purpouse only. remove it
unsigned char id[] = "id: ";// created for test purpouse only. remove it

/** P R I V A T E   P R O T O T Y P E   ************/
void send_message_uart_to_can(void);
void send_message_can_to_uart(void);
void process_internal_message(unsigned char message);


/**************************************************
* Function name		: void initialize_user(void)
*
* Created by		: Luca Lucci
* Date created		: 07/10/12
* Description		: user's initialization
* Notes				: -
**************************************************/
void initialize_user(void)
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

  // Init state machine
  uart_pc_interface_state = 0;
  uart_data_count = 0;
  can_pc_interface_state = 0;

  // Driver select
  uart_number = switch_sw6 + 1;

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

  uart_enabled = switch_sw7 + switch_sw8;
  if(uart_enabled)
  {
    //enable the driver
    if(uart_number == 1)
      MAX3160_ENABLE1();
    else
      MAX3160_ENABLE2();

    uart_init();
    uart_open(uart_baud_rate);
  }

  // initialize can bus

  // Init Can Bus Interface
  if(can_init(CAN_CONFIG, CAN_CONFIG_PROG, CAN_PROG_Bn_RX, CAN_1000kbps))
  {

    // Initialize can message
    if(switch_sw1)
    {
	  uart_message.id = 0x00;	// pc interface*/
      // Initialize Masks to accept all message
      can_init_mask_id(0, 0b0000000000, 0);
      can_init_mask_id(1, 0b0000000000, 0);
    }
    else
    {
  	  uart_message.id = cbrn_get_can_index();
      can_init_mask_id(0, 0b11111111111, 0);
      can_init_mask_id(1, 0b11111111111, 0);
    }

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

    /*for(i = 0; i < 8; i++)
      can_init_filter_buffer(i,i);
  */
    // Enable/disable buffer filters
    can_init_filter_enable(RXF_ALL_ON);
  }

    // initialize flag
    uart_message_queue = 0; 

}


/**************************************************
* Function name		: void uart_to_canbus_process(void)
*
* Created by		: Luca Lucci
* Date created		: 07/11/12
* Description		: function that has to be called in the main loop. It
*					  process uart and canbus message.
* Notes				: -
**************************************************/
void uart_to_canbus_process(void)
{
  send_message_uart_to_can();

  send_message_can_to_uart();
}

/**************************************************
* Function name		: void send_message_uart_to_can(void)
*
* Created by		: Luca Lucci
* Date created		: 05/12/12
* Description		: This function process message from uart and send it
*					  over can bus
* Notes				: When switch1 is on (pc interface mode) only valid frame
*					  will be passed through.
*					  Warning!! When switch1 is on the main loop will be slowed
*					  due to the additional information send through. This can
*					  bring to a can buffer rx overflow. In this situation the
*					  max buffer size value in can_interface.h have to be changed
**************************************************/
void send_message_uart_to_can(void)
{
  unsigned char data_ptr;	// for fill the data

  data_ptr = 0; //hold current data address in can message frame data array

  if(uart_interrupt_rx == 0)
    uart_buffer_rx_load(); // without interrupt I have to poll usart

  // I don't want to read anything from uart buffer before send the message in
  // queue
  if(uart_message_queue)
  {
    // if message has been sent clear the flag
    if(can_buffer_tx_load(&uart_message))
	  uart_message_queue = 0;
  }
  else 
  {
 	//**************************************************
    // check uart message received
	//**************************************************
    // Get data until buffer is empty or reach the maximum data that can be sent
	// in one can frame. Due to can frame format, the state machine store as much
	// data as possible from uart to fill all data portion
    while(!uartx_status->buffer_rx_empty && data_ptr < 8)
    {
      uart_buffer_read(&uart_message.data[data_ptr]); // read data from uart

	  if(switch_sw1)	//if pc interface is set then update the state machine
	  {
	    switch(uart_pc_interface_state)
		{
		  case 0:	//no message to send; waiting for header
		  	if(uart_message.data[data_ptr] == HEADER)
			  uart_pc_interface_state++;
			break;

		  case 1:	//read first index byte and send over can bus
			uart_message.id = ((unsigned long)uart_message.data[data_ptr] & 0x0007) << 8;
			uart_pc_interface_state++;
			break;

		  case 2:	//read second index byte and send over can bus	
			uart_message.id |= uart_message.data[data_ptr];
			uart_pc_interface_state++;
			break;

		  case 3:	//read length byte
			uart_data_count = uart_message.data[data_ptr];

			// If data length is zero then it is an internal message.
			if(uart_data_count == 0)
			  uart_pc_interface_state = 5;
			else
			  uart_pc_interface_state++;
			break;

		  case 4:	//send data over can bus
			data_ptr++;
			uart_data_count--;

			if(!uart_data_count)
			{
			  // data sent; return to initial state
			  uart_pc_interface_state = 0;
			}
			break;

		  case 5:
			process_internal_message(uart_message.data[data_ptr]);

			// return to initial state
			uart_pc_interface_state = 0;
			break;

		  default:	
			uart_pc_interface_state = 0;
			break;
		}
  	  }
 	  else	//else send rtr message 
        data_ptr++;
    }

	// send as many byte as possible in one shot
    if(data_ptr)
    {
	  // change led status when data has been received from device
      led_st1 = ~led_st1;

      //update length value
      uart_message.data_length = data_ptr;

	  // load can tx buffer. If there's no buffer left, it will be assert queue 
      // flag
	  if(!can_buffer_tx_load(&uart_message))
	    uart_message_queue = 1;
    }
  }

  // send uart message on can bus
  if(!can_status.buffer_tx_empty)
  {
  	can_buffer_send();
  }

}

/**************************************************
* Function name		: void send_message_can_to_uart(void)
*
* Created by		: Luca Lucci
* Date created		: 05/12/12
* Description		: This function process message from can and send it over
*					  uart.
* Notes				: When switch1 is off (sensor mode) only frame with the
*					  sensor id will be accepted.
**************************************************/
void send_message_can_to_uart(void)
{
  unsigned char return_value;	//store the value returned from macro

  //**************************************************
  // check can message received
  //**************************************************

  #ifndef CAN_INTERRUPT_RX
    //read the buffer that had received the new data
    can_buffer_rx_load();
  #endif

  // In this case I would like to send even small portion of data from a large
  // data frame. I need this state machine due to the uart buffer overflow 
  // event: every time I can't send message to uart I would like to try to send
  // the same byte.
  switch(can_pc_interface_state)
  {
    case 0:
	  //read the message from rx buffer
      if(can_buffer_read(&can_message))
	  {
  		// Check which type of command I have received. If it is a broad cast command
  		// or group command call the function to process internal message
		if((can_message.id == (uart_message.id & 0xF00)) || 
			(can_message.id == (uart_message.id & 0xFF0)))
		{
		  // If I have the command byte I can call 
		  // process_internal_message() function, otherwise I have to wait the
		  // next message

		  process_internal_message(can_message.data[0]);
		}
		else
		{
	      if(switch_sw1)
	        can_pc_interface_state++;
	      else
		  	can_pc_interface_state = 5;
		}
	  }
      break;

    case 1:	//send header
      uart_buffer_tx_load(return_value, HEADER);

	  if(return_value) 
	    can_pc_interface_state++;
	  else
		break;

	case 2:	//send first address byte
	  uart_buffer_tx_load(return_value,(can_message.id >> 8));

 	  if(return_value) 
	    can_pc_interface_state++;
 	  else
	    break;

	case 3:	//send second address byte and send over can bus
	  uart_buffer_tx_load(return_value, can_message.id);
		    
      if(return_value) 
	    can_pc_interface_state++;
	  else
		break;

	case 4:	//send length byte
	  uart_buffer_tx_load(return_value, can_message.data_length);

      if(return_value) 
	    can_pc_interface_state++;
	  else
		break;

	case 5:	//send data over uart
	  uart_buffer_tx_seq_load(return_value, (unsigned char *)&can_message.data, can_message.data_length);

      if(return_value) 
	  {
	    // change led status when data has been received from device
        led_st2 = ~led_st2;

	    can_pc_interface_state = 0;
	  }
	break;

	default:
	    uart_pc_interface_state = 0;
	  break;
  }
}

/**************************************************
* Function name		: void process_internal_message(unsigned char message)
*
* Created by		: Luca Lucci
* Date created		: 05/12/12
* Description		: This function process internal message from uart 
* Notes				: 0x00 - reset micro
*					  0x01 - set rs-232 baudrate at 9600
*					  0x02 - set rs-232 baudrate at 19200
*					  0x03 - set rs-232 baudrate at 38400
*					  0x04 - set rs-232 baudrate at 57600
*					  0x05 - set rs-232 baudrate at 115200
*					  0x06 - set can bus baudrate at 125kbps
*					  0x07 - set can bus baudrate at 250kbps
*					  0x08 - set can bus baudrate at 500kbps
*					  0x09 - set can bus baudrate at 1000kbps
*					  0x0A - enable terminator resistor
*					  0x0B - disable terminator resistor
**************************************************/
void process_internal_message(unsigned char message)
{
  switch(message)
  {
	case 0x00:	//reset
	  Reset();
	  break;
	case 0x01:	//set rs232 baudrate at 9600
	  uart_close();
	  uart_open(9600);
	  break;
	case 0x02:	//set rs232 baudrate at 19200
	  uart_close();
	  uart_open(19200);
	  break;
	case 0x03:	//set rs232 baudrate at 38400
	  uart_close();
	  uart_open(38400);
	  break;
	case 0x04:	//set rs232 baudrate at 57600
	  uart_close();
	  uart_open(57600);
	  break;
	case 0x05:	//set rs232 baudrate at 115200
	  uart_close();
	  uart_open(115200);
	  break;
	case 0x06:	//set can bus baudrate at 125kbps
	  can_init_baudrate_set(1, 3, 2, 2, FOSC_MHZ, CAN_125kbps);
	  break;
	case 0x07:	//set can bus baudrate at 250kbps
	  can_init_baudrate_set(1, 3, 2, 2, FOSC_MHZ, CAN_250kbps);
	  break;
	case 0x08:	//set can bus baudrate at 125kbps
	  can_init_baudrate_set(1, 3, 2, 2, FOSC_MHZ, CAN_500kbps);
	  break;
	case 0x09:	//set can bus baudrate at 125kbps
	  can_init_baudrate_set(1, 3, 2, 2, FOSC_MHZ, CAN_1000kbps);
	  break;
	case 0x0A:	//enable termination resistor
	  can_bus_enable_terminator;
	  break;
	case 0x0B:	//disable termination resistor
	  can_bus_disable_terminator;
	  break;
	default:
	  break;
  }
}
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
* function is can_uart_loop that must be called in the main loop. 
* This module send message over rs232 without any added data. If SW1 of dip
* switch is on, then the module add 1 byte header and 1 byte index and 1 byte 
* data length before the payload. This module also manage the rs485 control pin:
* this task can't be process by uart module because we must poll the TSR register.
*
* DIP SWITCH SW1-SW5: DEVICE INDEX
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
*   0x0C -> send info
*   0x10 -> start sensor
*   0x11 -> stop sensor
*   0x12 -> start acquisition
*   0x13 -> stop acquisition
*
* Configuration Notes:
*  - if set the clock at 16MHz with 38400 baud for uart1, polling uart_read it's
*    not enought. The interrupt on rx must be enable
****************************************************/

/** I N C L U D E S ********************************/
#include "can_to_rs232_converter.h"
#include "version.h"
#include <stdio.h>
#include <string.h>

/** V A R I A B L E S  *****************************/
CAN_MESSAGE can_message; 	//old can message arrived
CAN_MESSAGE uart_message;	//old uart message arrived

unsigned char device_type;	//store the type of the device
unsigned long pc_interface_address;

unsigned char uart_number;	//serial driver selection (1 or 2)
unsigned char uart_type;	//hardware flag to selected uart's type

// Variable for uart configuration. These have been decleared to abstract
// function to serial's number choose.
unsigned char uart_interrupt_tx;
unsigned char uart_interrupt_rx;
unsigned long uart_baud_rate;
struct uart_status *uartx_status;

unsigned char debug_interrupt_tx;

unsigned char error_message[100];
unsigned char error_message_length;

/** D E F I N E ************************************/

// Sync param to setup the can baudrate for a 10 m cable
/*SJW, PRSEG,PS1, PS2,brp*/
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
void can_uart_message_read_from_uart(void);
void can_uart_message_read_from_can(void);
void can_uart_proc_int_message(unsigned char cmd);
unsigned int cbrn_get_can_address(unsigned char device_type);
unsigned char pc_interface_uart_state_machine(unsigned char current_state);
unsigned char pc_interface_can_state_machine(unsigned char current_state);
unsigned char trasparent_uart_state_machine(unsigned char current_state);
unsigned char trasparent_can_state_machine(unsigned char current_state);
unsigned char sens_exp_uart_state_machine(unsigned char current_state);


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

  // define type of the device
  device_type = (switch_sw1_to_sw6) & 0x1F;

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

    debug_interrupt_tx = UART2_INTERRUPT_TX;
  }
  else
  {
    uartx_status = &uart2_status;
    uart_baud_rate = UART2_BAUD_RATE;
    uart_interrupt_tx = UART2_INTERRUPT_TX;
    uart_interrupt_rx = UART2_INTERRUPT_RX;

    debug_interrupt_tx = UART1_INTERRUPT_TX;
  }

  uart1_tx_tris = OUTPUT_PIN;
  uart1_rx_tris = INPUT_PIN;
  uart2_tx_tris = OUTPUT_PIN;
  uart2_rx_tris = INPUT_PIN;

  // Choose the uart type
  switch(device_type)
  {
    case ROBOTIC_ARM:
      MAX3160_ENABLE();
      MAX3160_ENABLE_RS485();
      uart_init(RS485);
      uart_open(115200);

      uart_type = 2;
      break;

    case VEHICLE:
    case SENSOR_BWA_SASS2300:
    case SENSOR_BWA_SASS3100:
    case SENSOR_CWA_TIC_CAM:
    case SENSOR_CWA_TIC_CHEMPRO100:
    case SENSOR_CWA_TIC_MULTIRAEPLUS:
    case SENSOR_RWA_ANPDR77:
    case SENSOR_EXP_SE138K:
      MAX3160_ENABLE();
      MAX3160_ENABLE_RS232();
      uart_init(RS232);
      uart_open(9600);

      //I would remember which uart I have choosen
      uart_type = 1;
      break;

    case SENSOR_RWA_IDENTIFINDER:
      MAX3160_ENABLE();
      MAX3160_ENABLE_RS232();
      uart_init(RS232);
      uart_open(38400);

      //I would remember which uart I have choosen
      uart_type = 1;
      break;

    case SENSOR_ON_BOARD_LASER_RANGE_FINDER:
      MAX3160_ENABLE();
      MAX3160_ENABLE_RS232();
      uart_init(RS232);
      uart_open(57600);

      //I would remember which uart I have choosen
      uart_type = 1;
      break;

    case PC_INTERFACE:
    case CUSTOM_DEVICE:
    default:
      // Only custom device and pc interface can choose the uart type

      uart_type = (switch_sw7 << 1) + switch_sw8;
      switch(uart_type)
      {
        case 0: //UART disable
          MAX3160_DISABLE();
          break;

        case 1: //RS232
          MAX3160_ENABLE();
          MAX3160_ENABLE_RS232();
          uart_init(RS232);
          uart_open(uart_baud_rate);
          break;

        case 2: //RS485
          MAX3160_ENABLE();
          MAX3160_ENABLE_RS485();
          uart_init(RS485);
          uart_open(uart_baud_rate);
          break;

        case 3: //RS422
          MAX3160_ENABLE();
          MAX3160_ENABLE_RS422();
          uart_init(RS232);
          uart_open(uart_baud_rate);
          break;
      }
      break;
  }
	  
  //init debug interface
  MAX3160_ENABLE_DEBUG();
  MAX3160_ENABLE_RS232_DEBUG();
  debug_init(RS232);
  debug_open(uart_baud_rate);

  // initialize can bus
  // Init Can Bus Interface
  if(can_init(CAN_MODE, CAN_CONFIG, CAN_CONFIG_PROG, CAN_PROG_Bn_RX, (unsigned char *) CAN_1000kbps))
  {
    pc_interface_address = cbrn_get_can_address(device_type);
    uart_message.id = pc_interface_address + CAN_ADDRESS_OFFSET;

    // Initialize can message
    if(device_type == PC_INTERFACE)
    {
      can_bus_enable_terminator;

      // Initialize Masks to accept all message
      can_init_mask_id(0, 0b0000000000, 0);
      can_init_mask_id(1, 0b0000000000, 0);
    }
    else
    {
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
    can_init_filter_id(0, pc_interface_address & 0xF00, 0);
    // Init filter 1 for group message
    can_init_filter_id(1, pc_interface_address & 0xFF0, 0);

    // Init filter 2 for dedicated internal message
    if(device_type != PC_INTERFACE)
      can_init_filter_id(2, pc_interface_address - 1, 0);
    else
      can_init_filter_id(2, pc_interface_address, 0);

    // Init other filter for normal message
    for(i = 3; i < 16; i++)
      can_init_filter_id(i, pc_interface_address, 0);

    // Associate filter to buffer.
    can_init_filter_buffer(0,0);
    can_init_filter_buffer(1,1);
    can_init_filter_buffer(2,1);
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

    //Send device type code
    if(device_type != PC_INTERFACE)
      can_uart_proc_int_message(0x0C);
  }
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
  unsigned char can_error;
  unsigned char uart_error;

  can_uart_message_read_from_uart();

  can_uart_message_read_from_can();

  can_buffer_send();

  // if interrupt is disabled then polling send function for uart
  if(!uart_interrupt_tx)
    uart_buffer_send();

  // Manage can error
  can_error = can_error_handle();
  if(can_error)
  {
     led_st3 = 1;
    //uart1_buffer_tx_load(can_error);
//    debug_buffer_tx_load(return_value, can_error);
//    debug_buffer_tx_load(return_value, 0x0D);
  }

  // Manage uart error
  uart_error_handle(uart_error);
  if(uart_error)
  {
    led_st3 = 1;
    //debug_buffer_tx_load(return_value, uart_error);
  }

  // if interrupt is disabled then polling send function for uart
  if(!debug_interrupt_tx)
    debug_buffer_send();

}

/**************************************************
* Function name		: void can_uart_message_read_from_uart(void)
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
void can_uart_message_read_from_uart(void)
{
  static unsigned char uart_message_read_state = 0;

  // I have to poll uart's TSR register to manage rs485 control pin, when 
  // enabled.
  if(uart_type == 2)
    uart_tsr_poll();

  if(uart_interrupt_rx == 0)
    uart_buffer_rx_load(); // without interrupt I have to poll usart

  switch(uart_message_read_state)
  {
    case 0:  //common task
      uart_message_read_state++;
      // if there's an action here, uncomment break
      //break;

    default:  //manage device
      switch(device_type)
      {
	case SENSOR_EXP_SE138K:
	  uart_message_read_state = sens_exp_uart_state_machine(uart_message_read_state);
	  break;

        case PC_INTERFACE:
	  uart_message_read_state = pc_interface_uart_state_machine(uart_message_read_state);
	  break;

        case VEHICLE:
        case ROBOTIC_ARM:
	case SENSOR_ON_BOARD_LASER_RANGE_FINDER:
        case SENSOR_RWA_IDENTIFINDER:
        case SENSOR_BWA_SASS2300:
        case SENSOR_BWA_SASS3100:
	case SENSOR_CWA_TIC_CAM:
	case SENSOR_CWA_TIC_CHEMPRO100:
        case SENSOR_CWA_TIC_MULTIRAEPLUS:
        case SENSOR_RWA_ANPDR77:
        case CUSTOM_DEVICE:
        default:
          uart_message_read_state = trasparent_uart_state_machine(uart_message_read_state);
          break;
      }
      break;
  }
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

      // Check which type of command I have received. If it is a broad cast command
      // or group command call the function to process internal message
      if((can_message.id == (pc_interface_address & 0xF00)) || 
         (can_message.id == (pc_interface_address & 0xFF0)) ||
         (can_message.id == (pc_interface_address - 1)))
      {
        if(can_message.data_length > 0)
          can_uart_proc_int_message(can_message.data[0]);
        break;
      }

      can_message_read_state++;
      break;

    default:  //manage device
      switch(device_type)
      {		
        case PC_INTERFACE:
	  can_message_read_state = pc_interface_can_state_machine(can_message_read_state);
	  break;

        case SENSOR_EXP_SE138K:
	  can_message_read_state = 0;
	  break;

        case VEHICLE:
        case ROBOTIC_ARM:
	case SENSOR_ON_BOARD_LASER_RANGE_FINDER:
        case SENSOR_RWA_IDENTIFINDER:
        case SENSOR_BWA_SASS2300:
        case SENSOR_BWA_SASS3100:
        case SENSOR_CWA_TIC_CAM:
        case SENSOR_CWA_TIC_CHEMPRO100:
        case SENSOR_CWA_TIC_MULTIRAEPLUS:
        case SENSOR_RWA_ANPDR77:
        case CUSTOM_DEVICE:
        default:
	  can_message_read_state = trasparent_can_state_machine(can_message_read_state);
	  break;
      }
      break;
  }  
}

/**************************************************
* Function name		: void can_uart_proc_int_message(unsigned char cmd)
*	cmd				: command to process
*   description		: string where to store the command executed
*	desc_length		: number of byte write in description
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
*   				  0x0C -> send info
*                     0x10 -> start sensor
*                     0x11 -> stop sensor
*                     0x12 -> start acquisition
*                     0x13 -> stop acquisition
**************************************************/
void can_uart_proc_int_message(unsigned char cmd)
{
  unsigned char result = 0;
  unsigned char buffer[30];                                                                               

  switch(cmd)
  {
	case RESET:	//reset
	  //*desc_length = sprintf(description, "Reset\r\n");

      Reset();
	  break;
	case SET_RS232_BAUD_9600:	//set rs232 baudrate at 9600
	  //*desc_length = sprintf(description, "Uart to 9600\r\n");

	  uart_close();
	  uart_open(9600);
	  break;
	case SET_RS232_BAUD_19200:	//set rs232 baudrate at 19200
	  //*desc_length = sprintf(description, "uart 9600");

	  uart_close();
	  uart_open(19200);
	  break;
	case SET_RS232_BAUD_38400:	//set rs232 baudrate at 38400
	  //*desc_length = sprintf(description, "uart 38400");

	  uart_close();
	  uart_open(38400);
	  break;
	case SET_RS232_BAUD_57600:	//set rs232 baudrate at 57600
	  //*desc_length = sprintf(description, "uart 57600");

	  uart_close();
	  uart_open(57600);
	  break;
	case SET_RS232_BAUD_115200:	//set rs232 baudrate at 115200
	  //*desc_length = sprintf(description, "uart 115200");

	  uart_close();
	  uart_open(115200);
	  break;
	case SET_CAN_BAUD_125:	//set can bus baudrate at 125kbps
	  //*desc_length = sprintf(description, "can 125kbps");

	  can_init_baudrate_set(CAN_125kbps[0], CAN_125kbps[1], CAN_125kbps[2], CAN_125kbps[3], CAN_125kbps[4]);
	  break;
	case SET_CAN_BAUD_250:	//set can bus baudrate at 250kbps
	  //*desc_length = sprintf(description, "can 250kbps");

	  can_init_baudrate_set(CAN_250kbps[0], CAN_250kbps[1], CAN_250kbps[2], CAN_250kbps[3], CAN_250kbps[4]);
	  break;
	case SET_CAN_BAUD_500:	//set can bus baudrate at 500kbps
	  //*desc_length = sprintf(description, "can 500kbps");

	  can_init_baudrate_set(CAN_500kbps[0], CAN_500kbps[1], CAN_500kbps[2], CAN_500kbps[3], CAN_500kbps[4]);
	  break;
	case SET_CAN_BAUD_1000:	//set can bus baudrate at 1000kbps
	  //*desc_length = sprintf(description, "can 1000kbps");

	  can_init_baudrate_set(CAN_1000kbps[0], CAN_1000kbps[1], CAN_1000kbps[2], CAN_1000kbps[3], CAN_1000kbps[4]);
	  break;
	case ENABLE_TERMINATOR:	//enable termination resistor
	  //*desc_length = sprintf(description, "Terminator enabled");

	  can_bus_enable_terminator;
	  break;
	case DISABLE_TERMINATOR:	//disable termination resistor
	  //*desc_length = sprintf(description, "Terminator disabled");

	  can_bus_disable_terminator;
	  break;
    case SEND_DEVICE_INFO:  //Send device info with the command supported
      uart_message.id -= 1;

      switch(device_type)
      {
        case SENSOR_BWA_SASS2300:
          uart_message.data_length = 6;
          uart_message.data[0] = 0x10 | cmd; //command recevied
          uart_message.data[1] = VERSION;
          uart_message.data[2] = START_SENSOR;
          uart_message.data[3] = STOP_SENSOR;
          uart_message.data[4] = START_ACQUISITION;
          uart_message.data[5] = STOP_ACQUISITION;

          //if(!can_buffer_tx_load(&uart_message))
          //  led_st3 = 1;
          can_buffer_tx_load(&uart_message);
	  break;

        case SENSOR_BWA_SASS3100:
          uart_message.data_length = 4;
          uart_message.data[0] = 0x10 | cmd; //command recevied
          uart_message.data[1] = VERSION;
          uart_message.data[2] = START_SENSOR;
          uart_message.data[3] = STOP_SENSOR;

          //if(!can_buffer_tx_load(&uart_message))
          //  led_st3 = 1;
          can_buffer_tx_load(&uart_message);
	  break;

        case VEHICLE:
        case ROBOTIC_ARM:
	case SENSOR_ON_BOARD_LASER_RANGE_FINDER:
        case SENSOR_RWA_IDENTIFINDER:
        case SENSOR_CWA_TIC_CAM:
        case SENSOR_CWA_TIC_CHEMPRO100:
        case SENSOR_CWA_TIC_MULTIRAEPLUS:
        case SENSOR_RWA_ANPDR77:
        case SENSOR_EXP_SE138K:
        case CUSTOM_DEVICE:
        default:
          uart_message.data_length = 2;
          uart_message.data[0] = 0x10 | cmd; //command recevied
          uart_message.data[1] = VERSION;

          //if(!can_buffer_tx_load(&uart_message))
          //  led_st3 = 1;

          can_buffer_tx_load(&uart_message);
          break;
	  }
      uart_message.id += 1;

      break;
    case START_SENSOR:
      switch(device_type)
      {
        case SENSOR_BWA_SASS2300:
        case SENSOR_BWA_SASS3100:
          sprintf(buffer, "#F1\r");
	  uart_buffer_tx_seq_load(result, buffer, strlen(buffer));
	  break;
      }
      break;
    case STOP_SENSOR:
      switch(device_type)
      {
	case SENSOR_BWA_SASS2300:
        case SENSOR_BWA_SASS3100:
          sprintf(buffer, "#F0\r");
	  uart_buffer_tx_seq_load(result, buffer, strlen(buffer));
	  break;
      }
      break;
    case START_ACQUISITION:
      switch(device_type)
      {
	case SENSOR_BWA_SASS2300:
          sprintf(buffer, "#G1\r");
	  uart_buffer_tx_seq_load(result, buffer, strlen(buffer));
	  break;
      }
      break;
    case STOP_ACQUISITION:
      switch(device_type)
      {
 	case SENSOR_BWA_SASS2300:
          sprintf(buffer, "#G0\r");
	  uart_buffer_tx_seq_load(result, buffer, strlen(buffer));
	  break;
      }
      break;

    default:
      //*desc_length = sprintf(description, "Command Unknown");
      break;
  }
}

/**************************************************
* Function name		: unsigned int cbrn_get_can_address(unsigned char device_type)
*	return			: 11 bit can index as stated in cbrn.h
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function convert dip switch code in can index 
* Notes				: Index Table:
*						0: 		pc interface
*						1-3: 	sensor on board
*						4-7: 	actuators (vehicle, robotic arm...)
*						8-16: 	cbrn sensors (CWA, RWA etc.)
*						17:	    pc interface
*						18-32:	custom device
**************************************************/
unsigned int cbrn_get_can_address(unsigned char device_type)
{
  switch(device_type)
  {
    case SENSOR_ON_BOARD_LASER_RANGE_FINDER:	//laser range finder
      return SENSOR_ON_BOARD_LASER_RANGE_FINDER_ADDR;
      break;
    case VEHICLE:	//vehicle
      return VEHICLE_ADDR;
      break;
    case ROBOTIC_ARM:	//robotic arm
      return ROBOTIC_ARM_ADDR;
      break;
    case SENSOR_BWA_SASS2300:	//sass 2300
      return SENSOR_BWA_SASS2300_ADDR;
      break;
    case SENSOR_BWA_SASS3100:	//sass 3100
      return SENSOR_BWA_SASS3100_ADDR;
      break;
    case SENSOR_CWA_TIC_CAM:	// CAM
      return SENSOR_CWA_TIC_CAM_ADDR;
      break;
    case SENSOR_CWA_TIC_CHEMPRO100:	//CHEMPRO100
      return SENSOR_CWA_TIC_CHEMPRO100_ADDR;
      break;
    case SENSOR_CWA_TIC_MULTIRAEPLUS:	// MULTIRAE PLUS
      return SENSOR_CWA_TIC_MULTIRAEPLUS_ADDR;
      break;
    case SENSOR_RWA_ANPDR77:	// ANPDR-77
      return SENSOR_RWA_ANPDR77_ADDR;
      break;
    case SENSOR_RWA_IDENTIFINDER:	//IdentiFINDER
      return SENSOR_RWA_IDENTIFINDER_ADDR;
      break;
    case SENSOR_EXP_SE138K:	// exp sensor
      return SENSOR_EXP_SE138K_ADDR;
      break;
    case PC_INTERFACE:
      return PC_INTERFACE_ADDR;
      break;
    default:	// CUSTOM OR UNKNOWN DEVICE
      if(device_type >= CUSTOM_DEVICE)
      {
	//return a progressive number starting on CUSTOM_DEVICE. Remember that
        //for each device there's an address for the one and the address for 
	return (UNKNOWN_DEVICE_ADDR - (((device_type & 0x0F) * 2) - 2));
      }
      else
        return UNKNOWN_DEVICE_ADDR;
      break;
  }
}

/**************************************************
* Function name		: unsigned char pc_interface_can_state_machine(unsigned char current_state)
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
unsigned char pc_interface_can_state_machine(unsigned char current_state)
{
  unsigned char return_value;	//store the value returned from macro

  // In this case I would like to send even small portion of data from a large
  // data frame. I need this state machine due to the uart buffer overflow 
  // event: every time I can't send message to uart I would like to try to send
  // the same byte.
  switch(current_state)
  {
    case 1:	//send header
      uart_buffer_tx_load(return_value, HEADER);

      if(return_value) 
        current_state++;
      else
        break;

	case 2:	//send message's length and first address byte
	  uart_buffer_tx_load(return_value,(can_message.data_length << 3)|(can_message.id >> 8));

 	  if(return_value) 
	    current_state++;
 	  else
        break;

	case 3:	//send second address byte and send over can bus
	  uart_buffer_tx_load(return_value, can_message.id);
		  
      if(return_value) 
	    current_state++;
	  else
        break;

	case 4:	//send data over uart
      uart_buffer_tx_seq_load(return_value, (unsigned char *)&can_message.data, can_message.data_length);

      if(return_value) 
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
* Function name		: unsigned char pc_interface_uart_state_machine(unsigned char current_state)
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
unsigned char pc_interface_uart_state_machine(unsigned char current_state)
{
  unsigned char return_value;	//store the value returned from macro
  unsigned char data_ptr = 0;	// for fill the data

  static unsigned char uart_data_count = 0;  //to count bytes received form usart

  //**************************************************
  // check uart message received
  //**************************************************
  // Get data until buffer is empty or reach the maximum data that can be sent
  // in one can frame. Due to can frame format, the state machine store as much
  // data as possible from uart to fill all data portion
  switch(current_state)
  {
    case 1:	//no message to send; waiting for header        
      uart_buffer_read(return_value, &uart_message.data[data_ptr]); // read data from uart
	  if(!return_value)
	    break;

      if(uart_message.data[data_ptr] == HEADER)
	    current_state++;
		
	  // stop if there's nothing to read into the next state
	  if(uartx_status->buffer_rx_empty) 
	    break;

	case 2:	//read the message's length and first index byte
      uart_buffer_read(return_value, &uart_message.data[data_ptr]); // read data from uart
	  if(!return_value)
	    break;

	  uart_data_count = uart_message.data[data_ptr] >> 3;

	  // If data length is zero then it is an internal message.
	  if(uart_data_count == 0)
	  {
	    current_state = 6;
		break;
      }   

	  uart_message.id = ((unsigned long)uart_message.data[data_ptr] & 0x0007) << 8;
	  current_state++;

	  // stop if there's nothing to read
	  if(uartx_status->buffer_rx_empty) 
	    break;

	case 3:	//read second index byte and send over can bus	
      uart_buffer_read(return_value, &uart_message.data[data_ptr]); // read data from uart
	  if(!return_value)
	    break;

	  uart_message.id |= uart_message.data[data_ptr];
	  current_state++;

	  // stop if there's nothing to read
	  if(uartx_status->buffer_rx_empty) 
	    break;

	case 4:	//send data over can bus
      while(!uartx_status->buffer_rx_empty && data_ptr < 8)
	  {
	    uart_buffer_read(return_value, &uart_message.data[data_ptr]); // read data from uart

        if(!return_value)
          break;

	    data_ptr++;
	    uart_data_count--;

	    if(!uart_data_count)
	      break;
      }
	  
	  if(data_ptr)
	  {
        //update length value
        uart_message.data_length = data_ptr;
        current_state++;
      }
	  else
	    break;

	//in this point don't take a break cause I want to send the message as soon
	//as possible and, if it fails, restart from here
         
    case 5:
      // load can tx buffer. If there's no buffer left, it will be assert queue 
      // flag
      if(!can_buffer_tx_load(&uart_message))
        break;

	  // It should be that I haven't read all data as stated in length byte.
	  // In this case I have to return into the previouse state
	  if(!uart_data_count)
	    current_state = 0;
	  else
	    current_state = 4;

      // change led status when data has been received from device
      led_st1 = ~led_st1;

      break;

	case 6:  //process internal message
	  uart_buffer_read(return_value, &uart_message.data[data_ptr]); // read data from uart
	  if(!return_value)
	  {
	    break;
	  }

	  can_uart_proc_int_message(uart_message.data[data_ptr]);
	  //uart_buffer_tx_seq_load(return_value);

	  // return to initial state
	  current_state = 0;
	  break;

	default:	
	  current_state = 0;
	  break;
  	
  }

  return current_state;
}

/**************************************************
* Function name		: unsigned char trasparent_can_state_machine(unsigned char current_state)
*	return			: future state
*	current_state	: the current state of the machine
*
* Created by		: Luca Lucci
* Date created		: 28/01/13
* Description		: This function process the can message for redirect it
*					  directly to uart
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char trasparent_can_state_machine(unsigned char current_state)
{
  unsigned char return_value;	//store the value returned from macro

  switch(current_state)
  {
    case 1:	//send data over uart
      uart_buffer_tx_seq_load(return_value, (unsigned char *)&can_message.data, can_message.data_length);
	  
      if(return_value) 
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
* Function name		: unsigned char trasparent_uart_state_machine(unsigned char current_state)
*	return			: future state
*	current_state	: the current state of the machine
*
* Created by		: Luca Lucci
* Date created		: 28/01/13
* Description		: This function process the uart message and redirect it 
*					  directly to can bus
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char trasparent_uart_state_machine(unsigned char current_state)
{
  unsigned char return_value;	//store the value returned from macro
  static CAN_MESSAGE message_from_uart;
  static unsigned char data_ptr = 0;	//take count of data read
  static unsigned int count = 0;
  static unsigned int count2 = 0;  //for test only

  switch(current_state)
  {
    case 1:	//read rs232 data
      //I want to use all the available byte in the frame. Since the uart data
      //is very slow respect the main loop, I introduce the count variable as 
      //the number of cycle to wait for collect data. If the frame is full
      //before count end, the function send data all the same.
      while((!uartx_status->buffer_rx_empty) && (data_ptr < 8))
	  {
        uart_buffer_read(return_value, &message_from_uart.data[data_ptr]); // read data from uart
        data_ptr++;
	  }
      
      // I start the counter only if I receive data but the buffer isn't full
      if((data_ptr > 0) && (data_ptr < 8))
        count++;

	  if(((data_ptr < 8) && (data_ptr > 0) && (count > 200)) || (data_ptr == 8))
	  {
        //update length value
        message_from_uart.id = (cbrn_get_can_address(device_type) + CAN_ADDRESS_OFFSET);
        message_from_uart.rtr_frame = 0;
        message_from_uart.exid_frame = 0;
        message_from_uart.data_length = data_ptr;

        count = 0;
        data_ptr = 0;
	    current_state++;
	  }
	  else
		break;

	  //don't put a break here!!!!

     case 2:
      if(!can_buffer_tx_load(&message_from_uart))
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


/**************************************************
* Function name		: unsigned char sens_exp_uart_state_machine(unsigned char current_state)
*	return			: future state
*	current_state	: the current state of the machine
*
* Created by		: Luca Lucci
* Date created		: 28/01/13
* Description		: This function process the uart message for the "sens_exp"
*				      device.
* Notes				: The case 0 is reserved to the calling function for common
*					  task. The function should return to the state 0
**************************************************/
unsigned char sens_exp_uart_state_machine(unsigned char current_state)
{
  //used in explosion matter sensor case
  static unsigned int cbrn_sens_exp_failure = 0;
  unsigned char result;	//store the value returned from macro

  switch(current_state)
  {
    case 1:	//send sensor code
      uart_buffer_tx_load(result, SENSOR_EXP_CODE);
	
	  if(result)
	    current_state++;
	  else
        led_st1 = 1;

	  break;

	case 2: //waiting for sensor code

      // If I don't receive the code then the sensor fail or has detected an
      // explosive gas
      if(cbrn_sens_exp_failure > 1000)
	  {
	    current_state++;
		break;
	  }

      // Check if I receive data; if not, I increment failure flag
      if(uartx_status->buffer_rx_empty)
	  {
        cbrn_sens_exp_failure++;
		current_state = 0;
		break;
	  }
 
      uart_buffer_read(result, &uart_message.data[0]); // read data from uart

      if(uart_message.data[0] == SENSOR_EXP_CODE)
      {
	    cbrn_sens_exp_failure = 0;
        current_state = 0;
		break;
	  }
      else
        cbrn_sens_exp_failure++;
 
	  break;

	case 3: //send warning      
      uart_message.data[0] = SENSOR_EXP_WARNING;
	  uart_message.data_length = 1;

      cbrn_sens_exp_failure = 0;

      // change led status when data has been received from device
      led_st1 = ~led_st1;

      // load can tx buffer
      if(!can_buffer_tx_load(&uart_message))
        break;
	
	  current_state = 0;
	  break;

    default:
      current_state = 0;
      break;
  }

  return current_state;
}

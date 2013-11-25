/***************************************************
* FileName:        can_to_rs232_converter.h
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
* Define prototype and const for can_to_rs232_converter.h
*
****************************************************/

#ifndef _CAN_TO_RS232_CONVERTER
#define _CAN_TO_RS232_CONVERTER

/** I N C L U D E S ********************************/
#include "uart_interface.h"
#include "can_interface.h"
#include "io_cfg.h"

/** V A R I A B L E S  *****************************/
extern unsigned char uart_number;	// number of serial driver used
extern unsigned char uart_interrupt_tx;
extern unsigned char uart_interrupt_rx;
extern unsigned long uart_baud_rate;
extern struct uart_status * uartx_status;	// pointer to the uart_status structur

/** D E F I N E ************************************/
// Header's value to send message to/from pc interface
#define HEADER	'$'

// Standard CAN index. 0x7FF is the maximum index allowed. 
//
// Warning: if someone add or remove a table index, the function 
// cbrn_get_can_address must be changed too
#define PC_INTERFACE_ADDR					0x000

#define SENSOR_ON_BOARD_LASER_RANGE_FINDER_ADDR	0x4FF

// This index is based on Segway RMP. Indexes from 0x500 throught 0x560 should 
// be keep free.
#define VEHICLE_ADDR				0x500	

#define ROBOTIC_ARM_ADDR			0x570
#define ROBOTIC_ARM_G1_ADRR			0x571
#define ROBOTIC_ARM_G2_ADRR			0x572
#define ROBOTIC_ARM_G3_ADRR			0x573
#define ROBOTIC_ARM_G4_ADRR			0x574
#define ROBOTIC_ARM_G5_ADRR			0x575
#define ROBOTIC_ARM_G6_ADRR			0x576
#define ROBOTIC_ARM_GRIPPER_ADRR	0x571

#define SENSOR_ALL_ADDR						0x600
#define SENSOR_BWA_ADDR						0x610
#define SENSOR_BWA_SASS2300_ADDR			0x611
#define SENSOR_CWA_TIC_ADDR					0x620
#define SENSOR_CWA_TIC_CAM_ADDR				0x621
#define SENSOR_CWA_TIC_CHEMPRO100_ADDR		0x622
#define SENSOR_CWA_TIC_MULTIRAEPLUS_ADDR	0x623
#define SENSOR_RWA_ADDR						0x630
#define SENSOR_RWA_ANPDR77_ADDR				0x631
#define SENSOR_RWA_IDENTIFINDER_ADDR		0x632
#define SENSOR_EXP_ADDR						0x640
#define UNKNOWN_DEVICE						0x7FF

// Device's define
#define SENSOR_ON_BOARD_LASER_RANGE_FINDER	0x00
#define VEHICLE								0x04	
#define ROBOTIC_ARM							0x05
#define SENSOR_BWA_SASS2300					0x08
#define SENSOR_CWA_TIC_CAM					0x09
#define SENSOR_CWA_TIC_CHEMPRO100			0x0A
#define SENSOR_CWA_TIC_MULTIRAEPLUS			0x0B
#define SENSOR_RWA_ANPDR77					0x0C
#define SENSOR_RWA_IDENTIFINDER			    0x0D
#define SENSOR_EXP							0x0E
#define PC_INTERFACE						0x10
#define CUSTOM_DEVICE						0x11

#define SENSOR_EXP_CODE				0xFF
#define SENSOR_EXP_WARNING			0x00


/** M A C R O **************************************/
// I don't want to worry about number of serial port used, so I define generic 
// function
#define uart_init(rs485_enable)	{if(uart_number == 1) uart1_init(rs485_enable); else uart2_init(rs485_enable);}
#define uart_open(baud_rate) {if(uart_number == 1) uart1_open(baud_rate); else uart2_open(baud_rate);}
#define uart_close() {if(uart_number == 1) uart1_close(); else uart2_close();}
#define uart_buffer_tx_load(return_value, byte) {if(uart_number == 1) return_value = uart1_buffer_tx_load(byte); else return_value = uart2_buffer_tx_load(byte);}
#define uart_buffer_tx_seq_load(return_value, data_write, length) {if(uart_number == 1) return_value = uart1_buffer_tx_seq_load(data_write, length); else return_value = uart2_buffer_tx_seq_load(data_write, length);}
#define uart_get_tx_buffer_empty_space(return_value) {if(uart_number == 1) return_value = uart1_get_tx_buffer_empty_space(); else return_value = uart2_get_tx_buffer_empty_space();}
#define uart_buffer_rx_load() {if(uart_number == 1) uart1_buffer_rx_load(); else uart2_buffer_rx_load();}
#define uart_buffer_read(return_value, byte) {if(uart_number == 1) return_value = uart1_buffer_read(byte); else return_value = uart2_buffer_read(byte);}
#define uart_tsr_poll() {if(uart_number == 1) uart1_tsr_poll(); else uart2_tsr_poll();}
#define uart_isr() {if(uart_number == 1) uart1_isr(); else uart2_isr();}
#define uart_buffer_send() {if(uart_number == 1) uart1_buffer_send(); else uart2_buffer_send();}
#define uart_error_handle(return_value, error_message, length) {if(uart_number == 1) return_value = uart1_error_handle(error_message, length); else return_value = uart2_error_handle(error_message, length);}

#define debug_init(rs485_enable)	{if(uart_number == 1) uart2_init(rs485_enable); else uart1_init(rs485_enable);}
#define debug_open(baud_rate) {if(uart_number == 1) uart2_open(baud_rate); else uart1_open(baud_rate);}
#define debug_close() {if(uart_number == 1) uart2_close(); else uart1_close();}
#define debug_buffer_tx_load(return_value, byte) {if(uart_number == 1) return_value = uart2_buffer_tx_load(byte); else return_value = uart1_buffer_tx_load(byte);}
#define debug_buffer_tx_seq_load(return_value, data_write, length) {if(uart_number == 1) return_value = uart2_buffer_tx_seq_load(data_write, length); else return_value = uart1_buffer_tx_seq_load(data_write, length);}
#define debug_isr() {if(uart_number == 1) uart2_isr(); else uart1_isr();}
#define debug_buffer_send() {if(uart_number == 1) uart2_buffer_send(); else uart1_buffer_send();}

#define MAX3160_ENABLE() {if(uart_number == 1) max3160_shdn1 = 1; else if(uart_number == 2) max3160_shdn2 = 1;}
#define MAX3160_DISABLE() {if(uart_number == 1) max3160_shdn1 = 0; else if(uart_number == 2) max3160_shdn2 = 0;}
#define MAX3160_ENABLE_RS232()	{if(uart_number == 1) MAX3160_ENABLE_RS232_1() else if(uart_number == 2) MAX3160_ENABLE_RS232_2()}
#define MAX3160_ENABLE_RS485()	{if(uart_number == 1)MAX3160_ENABLE_RS485_1() else if(uart_number == 2) MAX3160_ENABLE_RS485_2()}
#define MAX3160_ENABLE_RS422()  {if(uart_number == 1) MAX3160_ENABLE_RS422_1() else if(uart_number == 2) MAX3160_ENABLE_RS422_2()}

#define MAX3160_ENABLE_DEBUG() {if(uart_number == 1) max3160_shdn2 = 1; else if(uart_number == 2) max3160_shdn1 = 1;}
#define MAX3160_DISABLE_DEBUG() {if(uart_number == 1) max3160_shdn2 = 0; else if(uart_number == 2) max3160_shdn1 = 0;}
#define MAX3160_ENABLE_RS232_DEBUG()	{if(uart_number == 1) MAX3160_ENABLE_RS232_2() else if(uart_number == 2) MAX3160_ENABLE_RS232_1()}
#define MAX3160_ENABLE_RS485_DEBUG()	{if(uart_number == 1)MAX3160_ENABLE_RS485_2() else if(uart_number == 2) MAX3160_ENABLE_RS485_1()}
#define MAX3160_ENABLE_RS422_DEBUG()  {if(uart_number == 1) MAX3160_ENABLE_RS422_2() else if(uart_number == 2) MAX3160_ENABLE_RS422_1()}


/** S T R U C T  ***********************************/

/** P R O T O T Y P E S ****************************/
void can_uart_initialize(void);		// user's application initializations
void can_uart_loop(void);	// process uart and canbus  message

#endif


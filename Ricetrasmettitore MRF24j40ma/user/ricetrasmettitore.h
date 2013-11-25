/***************************************************
* FileName:        ricetrasmettitore.h
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
* Define prototype and const for ricetrasmettitore.h
*
****************************************************/

#ifndef _RICETRASMETTITORE
#define _RICETRASMETTITORE

/** I N C L U D E S ********************************/
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


/** S T R U C T  ***********************************/

/** P R O T O T Y P E S ****************************/
void can_spi_initialize(void);		// user's application initializations
void can_spi_loop(void);	// process uart and canbus  message


#endif


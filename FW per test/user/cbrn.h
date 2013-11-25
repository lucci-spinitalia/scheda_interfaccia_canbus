/***************************************************
* FileName:        cbrn.h
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
* First written on 26/11/2012 by Luca Lucci.
*
* Module Description:
* Define prototype and const for cbrn.h
*
****************************************************/

#ifndef _CBRN_H

#define _CBRN_H

/** D E F I N E ************************************/
// Standard CAN index. 0x7FF is the maximum index allowed

#define SENSOR_ON_BOARD_LASER_RANGE_FINDER	0x4FF

// This index is based on Segway RMP. Indexes from 0x500 throught 0x560 should 
// be keep free.
#define VEHICLE				0x500	

#define ROBOTIC_ARM			0x570
#define ROBOTIC_ARM_G1		0x571
#define ROBOTIC_ARM_G2		0x572
#define ROBOTIC_ARM_G3		0x573
#define ROBOTIC_ARM_G4		0x574
#define ROBOTIC_ARM_G5		0x575
#define ROBOTIC_ARM_G6		0x576
#define ROBOTIC_ARM_GRIPPER	0x571

#define SENSOR_ALL					0x600
#define SENSOR_BWA					0x610
#define SENSOR_BWA_SASS2300			0x611
#define SENSOR_CWA_TIC				0x620
#define SENSOR_CWA_TIC_CAM			0x621
#define SENSOR_CWA_TIC_CAMPRO100	0x622
#define SENSOR_CWA_TIC_MULTIRAEPLUS	0x623
#define SENSOR_RWA					0x630
#define SENSOR_RWA_ANPDR77			0x631
#define SENSOR_RWA_IDENTIFINDER		0x632
#define SENSOR_EXP					0x640

/** P R O T O T Y P E S ****************************/
unsigned int cbrn_get_can_index(void);

#endif
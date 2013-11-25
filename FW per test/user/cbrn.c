/***************************************************
* FileName:        cbrn.c
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
* This module provide convertion between dip switch index and can index.
* Also, it provides function to manage different device
*
****************************************************/

/** I N C L U D E S ********************************/
#include "cbrn.h"

/**************************************************
* Function name		: unsigned int cbrn_get_can_index(void)
*	return			: 11 bit can index as stated in cbrn.h
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function convert dip switch code in can index 
* Notes				: Index Table:
*						0-3: sensor on board
*						4-7: actuators (vehicle, robotic arm...)
*						8-16: cbrn sensors (CWA, RWA etc.)
**************************************************/
unsigned int cbrn_get_can_index(void)
{
  //number from dip switch selection that indicates type of target device
  unsigned char device_type;

  device_type = (switch_sw1_to_sw6 >> 1) & 0x0F;

  switch(device_type)
  {
	case 0:
	  return SENSOR_ON_BOARD_LASER_RANGE_FINDER;
	  break;
    case 4:
	  return VEHICLE;
	  break;
	case 5:
	  return ROBOTIC_ARM:
	  break;
	case 8:
	  return SENSOR_BWA_SASS2300;
	  break;
	case 9:
	  return SENSOR_CWA_TIC_CAM;
  	  break;
	case 10:
	  return SENSOR_CWA_TIC_CAMPRO100;
	  break;
	case 11:
	  return SENSOR_CWA_TIC_MULTIRAEPLUS;
	  break;
	case 12:
	  return SENSOR_RWA_ANPDR77;
	  break;
	case 13:
	  return SENSOR_RWA_IDENTIFINDER;
	  break;
	case 14:
	  return SENSOR_EXP;
	  break;
	default:	// UNKOWN DEVICE
	  return 0x4FF;
	  break
  }
}
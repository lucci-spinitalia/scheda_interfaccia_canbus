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
#include "../system/io_cfg.h"

/** V A R I A B L E S  *****************************/
rom unsigned char device_name_rom[20];	//buffer where store the device name
										//requested
/**************************************************
* Function name		: unsigned int cbrn_get_can_index(void)
*	return			: 11 bit can index as stated in cbrn.h
* Created by		: Luca Lucci
* Date created		: 31/10/12
* Description		: This function convert dip switch code in can index 
* Notes				: Index Table:
*						0: 		pc interface
*						1-3: 	sensor on board
*						4-7: 	actuators (vehicle, robotic arm...)
*						8-16: 	cbrn sensors (CWA, RWA etc.)
**************************************************/
unsigned int cbrn_get_can_index(void)
{
  //number from dip switch selection that indicates type of target device
  unsigned char device_type;

  device_type = (switch_sw1_to_sw6 >> 1) & 0x0F;

  switch(device_type)
  {
	case 0:	//laser range finder
	  return SENSOR_ON_BOARD_LASER_RANGE_FINDER;
	  break;
    case 4:	//vehicle
	  return VEHICLE;
	  break;
	case 5:	//robotic arm
	  return ROBOTIC_ARM;
	  break;
	case 8:	//sass 2300
	  return SENSOR_BWA_SASS2300;
	  break;
	case 9:	// CAM
	  return SENSOR_CWA_TIC_CAM;
  	  break;
	case 10:	//CHEMPRO100
	  return SENSOR_CWA_TIC_CHEMPRO100;
	  break;
	case 11:	// MULTIRAE PLUS
	  return SENSOR_CWA_TIC_MULTIRAEPLUS;
	  break;
	case 12:	// ANPDR-77
	  return SENSOR_RWA_ANPDR77;
	  break;
	case 13:	//IdentiFINDER
	  return SENSOR_RWA_IDENTIFINDER;
	  break;
	case 14:	// exp sensor
	  return SENSOR_EXP;
	  break;
	default:	// UNKOWN DEVICE
	  return 0x7FF;
	  break;
  }
}

/**************************************************
* Function name		: unsigned int cbrn_get_can_info(unsigned char *device_name)
*	return			: number of character
*	device_name		: device's name
* Created by		: Luca Lucci
* Date created		: 17/12/12
* Description		: This function return a pointer to a string that store the
*					 device name
* Notes				: Index Table:
*						0: 		pc interface
*						1-3: 	sensor on board
*						4-7: 	actuators (vehicle, robotic arm...)
*						8-16: 	cbrn sensors (CWA, RWA etc.)
**************************************************/
unsigned int cbrn_get_can_info(unsigned char *device_name)
{
  //number from dip switch selection that indicates type of target device
  unsigned char device_type;

  device_type = (switch_sw1_to_sw6 >> 1) & 0x0F;

  switch(device_type)
  {
    case 0:	//laser range finder
	  if(switch_sw1)
	  {
	    device_name[12] = 'P'; device_name[11] = 'C'; device_name[10] = ' ';
	    device_name[9] = 'I'; device_name[8] = 'N'; device_name[7] = 'T';
	    device_name[6] = 'E'; device_name[5] = 'R'; device_name[4] = 'F';
	    device_name[3] = 'A'; device_name[2] = 'C'; device_name[1] = 'E';
		device_name[0] = 0x0D;
		return 13;
	  }
	  else
	  {
	    device_name[18] = 'L'; device_name[17] = 'A'; device_name[16] = 'S';
	    device_name[15] = 'E'; device_name[14] = 'R'; device_name[13] = ' ';
	    device_name[12] = 'R'; device_name[11] = 'A'; device_name[10] = 'N';
	    device_name[9] = 'G'; device_name[8] = 'E'; device_name[7] = ' ';
	    device_name[6] = 'F'; device_name[5] = 'I'; device_name[4] = 'N';
	    device_name[3] = 'D'; device_name[2] = 'E'; device_name[1] = 'R';
		device_name[0] = 0xD;
	    return 19;
	  }
	  break;
    case 4:	//vehicle
	  device_name[7] = 'V'; device_name[6] = 'E'; device_name[5] = 'H';
	  device_name[4] = 'I'; device_name[3] = 'C'; device_name[2] = 'L';
	  device_name[1] = 'E'; device_name[0] = 0x0D;
	  return 8;
	  break;
	case 5:	//robotic arm
	  device_name[11] = 'R'; device_name[10] = 'O'; device_name[9] = 'B';
	  device_name[8] = 'O'; device_name[7] = 'T'; device_name[6] = 'I';
	  device_name[5] = 'C'; device_name[4] = ' '; device_name[3] = 'A';
	  device_name[2] = 'R'; device_name[1] = 'M'; device_name[0] = 0x0D;
	  return 12;
	  break;
	case 8:	//sass 2300
	  device_name[9] = 'S'; device_name[8] = 'A'; device_name[7] = 'S';
	  device_name[6] = 'S'; device_name[5] = ' '; device_name[4] = '2';
	  device_name[3] = '3'; device_name[2] = '0'; device_name[1] = '0';
      device_name[0] = 0x0D;
	  return 10;
	  break;
	case 9:	// CAM
	  device_name[3] = 'C'; device_name[2] = 'A'; device_name[1] = 'M';
      device_name[0] = 0x0D;
	  return 4;
  	  break;
	case 10:	//CHEMPRO100
	  device_name[10] = 'C'; device_name[9] = 'H'; device_name[8] = 'E';
	  device_name[7] = 'M'; device_name[6] = 'P'; device_name[5] = 'R';
	  device_name[4] = 'O'; device_name[3] = '1'; device_name[2] = '0';
	  device_name[1] = '0'; device_name[0] = '0';
	  return 11;
	  break;
	case 11:	// MULTIRAE PLUS
	  device_name[13] = 'M'; device_name[12] = 'U'; device_name[11] = 'L';
	  device_name[10] = 'T'; device_name[9] = 'I'; device_name[8] = 'R';
	  device_name[7] = 'A'; device_name[6] = 'E'; device_name[5] = ' ';
	  device_name[4] = 'P'; device_name[3] = 'L'; device_name[2] = 'U';
	  device_name[1] = 'S'; device_name[0] = 0x0D; 
	  return 14;
	  break;
	case 12:	// ANPDR-77
	  device_name[8] = 'A'; device_name[7] = 'N'; device_name[6] = 'P';
	  device_name[5] = 'D'; device_name[4] = 'R'; device_name[3] = '-';
	  device_name[2] = '7'; device_name[1] = '7'; device_name[0] = 0x0D; 
	  return 9;
	  break;
	case 13:	//IdentiFINDER
	  device_name[12] = 'I'; device_name[11] = 'D'; device_name[10] = 'E';
	  device_name[9] = 'N'; device_name[8] = 'T'; device_name[7] = 'I';
	  device_name[6] = 'F'; device_name[5] = 'I'; device_name[4] = 'N';
	  device_name[3] = 'D'; device_name[2] = 'E'; device_name[1] = 'R';
	  device_name[0] = 0x0D;
	  return 13;
	  break;
	case 14:	// exp sensor
	  device_name[10] = 'E'; device_name[9] = 'X'; device_name[8] = 'P';
	  device_name[7] = ' '; device_name[6] = 'S'; device_name[5] = 'E';
	  device_name[4] = 'N'; device_name[3] = 'S'; device_name[2] = 'O';
	  device_name[1] = 'R'; device_name[0] = 0x0D; 
	  return 11;
	  break;
	default:	// UNKOWN DEVICE
	  break;
  }
}
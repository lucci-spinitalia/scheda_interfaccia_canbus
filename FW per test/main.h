/***************************************************
* FileName:        main.h
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
* First written on 09/11/2012 by Luca Lucci.
*
* Module Description:
* Define prototype and const for main.c
*
****************************************************/

#ifndef _MAIN_H
#define _MAIN_H

/** D E F I N E ************************************/

/** P R I V A T E  P R O T O T Y P E S *************/
void main(void);	//Main program entry point
void initialize_system(void);	// default hardware initialization (switchs,
								// leds, buttons

#endif // _MAIN_H
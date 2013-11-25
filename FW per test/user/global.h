/***************************************************
* FileName:        global.h
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
* First written on 07/10/2012 by Luca Lucci.
*
* Module Description:
* Here it's defined all global variable
*
****************************************************/

#ifndef _GLOBAL_H
#define _GLOBAL_H

/** V A R I A B L E S ******************************/
extern unsigned char data[50];
extern unsigned int size_data;
extern unsigned int num_can_message;
extern unsigned int timer_preload;

void timer0_reload(unsigned int timer_preload);
#endif //_GLOBAL_H

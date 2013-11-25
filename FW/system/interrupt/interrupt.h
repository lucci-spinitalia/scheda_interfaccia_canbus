/***************************************************
* FileName:        interrupt.h
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
* Define prototype and const for interrupt.c
*
****************************************************/
#ifndef INTERRUPT_H
#define INTERRUPT_H

/** D E F I N I T I O N S ****************************************************/

/** S T R U C T U R E S ******************************************************/

/** P R O T O T Y P E S ******************************************************/
void low_isr(void);		// Manage low priority interrupts
void high_isr(void);	// Manage high priority interrupts

#endif //INTERRUPT_H
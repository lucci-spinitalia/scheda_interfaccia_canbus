/***************************************************
* FileName:        interrupt.c
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
* Definition of interrupt routines
*
****************************************************/

/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include "interrupt.h"
#include "..\io_cfg.h"
#include "..\..\user\can_to_rs232_converter.h"
#include <timers.h>

/** V A R I A B L E S ********************************************************/
unsigned char i;					 // created for test purpouse only. remove it

/** I N T E R R U P T  V E C T O R S *****************************************/

#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
    _asm goto  high_isr _endasm
}
#pragma code


#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
    _asm goto low_isr _endasm
}

/** D E C L A R A T I O N S **************************************************/


#pragma code

#pragma interrupt high_isr

/**************************************************
* Function name		: void high_isr(void)
*
* Created by		: Luca Lucci
* Date created		: 18/10/12
* Description		: Managment for high priority interrupt
* Notes				: -
**************************************************/
void high_isr(void)
{
  unsigned int timerl;
  unsigned int timerh;

  if(INTCONbits.TMR0IF)
  {
    INTCONbits.TMR0IE = 0;	//clear int flag
    INTCONbits.TMR0IF = 0;	//clear int flag

    led_st1 = ~led_st1;

    WriteTimer0(0xbdc);
    // il timer � impostato ad 1 secondo
    /*timerl = TMR0L;
    timerh = TMR0H;
    TMR0L = 0xdc;
    TMR0H = 0xb;*/

    INTCONbits.TMR0IE = 1;	//clear int flag
    //T0CONbits.TMR0ON = 1;
  }

  if( INTCONbits.INT0IF )
  {	
    /* Note: the interrupt latency will be three to four instruction cycles. The 
     * exact latency is the same for one or two-cycle instructions. Individual
     * interrupt flag bits are set regardless of the status of their 
     * corresponding enable bit or the GIE bit. 
     * There is no priority bit associated with INT0; it always a high priority
     * interrupt source */

    INTCONbits.INT0IF = 0;	// Required to reset in software before re-enable
	
  }
}


#pragma interruptlow low_isr

/**************************************************
* Function name		: void main(void)
*
* Created by		: Luca Lucci
* Date created		: 18/10/12
* Description		: Managment for low priority interrupt
* Notes				: -
**************************************************/
void low_isr(void)
{
}
#pragma code

/** EOF interrupt.c **********************************************************/

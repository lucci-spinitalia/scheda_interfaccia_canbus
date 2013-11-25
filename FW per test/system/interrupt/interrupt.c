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
* First written on 09/11/2012 by Luca Lucci.
*
* Module Description:
* Definition of interrupt routines
*
****************************************************/

/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include "..\..\user\global.h"
#include "..\..\user\uart_interface.h"
#include "..\io_cfg.h"
#include "interrupt.h"

/** V A R I A B L E S ********************************************************/

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
  static volatile unsigned int counter = 0;
  
  if(INTCONbits.TMR0IF)
  {
    INTCONbits.TMR0IF = 0;	//clear int flag
	
    interrupt_flag = 1;

	//uart1_buffer_tx_seq_load(data, size_data);
    uart1_buffer_tx_load(counter);
    timer0_reload(timer_preload);

    counter++;

    if(counter >= num_can_message)
    {
      counter = 0;
      INTCONbits.TMR0IE = 0;
    }
    //else
    // INTCONbits.TMR0IE = 1;

    interrupt_flag = 0;
  }


  if(INTCONbits.INT0IF)
  {	
    led_d0 = ~led_d0;
    INTCONbits.INT0F = 0;
	INTCONbits.TMR0IE = ~INTCONbits.TMR0IE;
  }

  uart1_isr();
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

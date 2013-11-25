/***************************************************
* FileName:        io_cfg.h
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
* First written on 16/10/2012 by Luca Lucci.
*
* Module Description:
* Here it's defined the hardware configuration
*
****************************************************/

#ifndef _IO_CFG_H
#define _IO_CFG_H

/** I N C L U D E S ********************************/

/** O S C I L L A T O R  ***************************/
#define CRYSTAL_FREQUENCY_MHZ	10
#if 1//(CONFIG1Hbits.PLLCFG == 1)// && (CONFIG1Hbits.FOSC != 8) && (CONFIG1Hbits.FOSC != 9)
  #define FOSC_MHZ CRYSTAL_FREQUENCY_MHZ*4
#else
  #define FOSC_MHZ CRYSTAL_FREQUENCY_MHZ
#endif


/** T R I S         ********************************/
#define INPUT_PIN	1
#define OUTPUT_PIN 	0

/** U S A R T       ********************************/
#define uart1_tx_tris	TRISCbits.TRISC6
#define uart1_rx_tris	TRISCbits.TRISC7

/** P I N           ********************************/
#define interrupt_tris	TRISAbits.TRISA5
#define interrupt_flag	LATAbits.LATA5

/** L E D           ********************************/
#define tris_LED_D7	      	TRISDbits.TRISD7    // Output
#define led_d7         		LATDbits.LATD7

#define tris_LED_D6		    TRISDbits.TRISD6    // Output
#define led_d6         		LATDbits.LATD6

#define tris_LED_D5		    TRISDbits.TRISD5    // Output
#define led_d5         		LATDbits.LATD5

#define tris_LED_D4		    TRISDbits.TRISD4    // Output
#define led_d4         		LATDbits.LATD4

#define tris_LED_D3         TRISDbits.TRISD3   // Output
#define led_d3         		LATDbits.LATD3

#define tris_LED_D2         TRISDbits.TRISD2    // Output
#define led_d2         		LATDbits.LATD2

#define tris_LED_D1         TRISDbits.TRISD1    // Output
#define led_d1         		LATDbits.LATD1

#define tris_LED_D0         TRISDbits.TRISD0    // Output
#define led_d0         		LATDbits.LATD0

/** B U T T O N     ********************************/
#define button_tris		TRISBbits.TRISB0
#define button			PORTBbits.RB0


#endif // _IO_CFG_H

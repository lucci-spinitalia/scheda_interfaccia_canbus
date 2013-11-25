/***************************************************
* FileName:        io_cfg.h
* Dependencies:    See INCLUDES section below
* Processor:       PIC18
* Compiler:        C18 3.30
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
#include <usart.h>

/** T R I S         ********************************/
#define INPUT_PIN	1
#define OUTPUT_PIN 	0

/** U S A R T       ********************************/
#define serial_port1_tx_tris	TRISCbits.TRISC6
#define serial_port1_rx_tris	TRISCbits.TRISC7

#define serial_port2_tx_tris	TRISDbits.TRISD6
#define serial_port2_rx_tris	TRISDbits.TRISD7

#define BAUD_CONFIG BAUD_16_BIT_RATE & BAUD_AUTO_OFF & BAUD_WAKEUP_OFF
#define USART_CONFIG USART_TX_INT_OFF & USART_RX_INT_ON & USART_BRGH_HIGH & USART_CONT_RX & USART_EIGHT_BIT & USART_ASYNCH_MODE

/* Baud rate BR with Sync = 0
*  BRG16    BRGH    MODE          BR FORMULA
*    0       0      8 bit/Async    Fosc/[64([SPBRGHx:SPBRGx]+1)]
*    0       1      8 bit/Async    Fosc/[16([SPBRGHx:SPBRGx]+1)]
*    1       0      16 bit/Async   Fosc/[16([SPBRGHx:SPBRGx]+1)]
*    1       1      16 bit/Async   Fosc/[4([SPBRGHx:SPBRGx]+1)]
*
* In Fosc must be taken in account the pll, too. With a 16MHz oscillator and a 
* 4x pll enabled, the reference Fosc in table 22-4 is 64 MHz
*	6666	// 2400	
*	1666	// 9600
*	832		// 19200
*	278		// 57600
*	138		// 115200
*/

#define BAUD_RATE 138	// 115200



/** C A N B U S     ********************************/
#define can_bus_tx_tris		TRISBbits.TRISB2

#define can_bus_rx_tris 	TRISBbits.TRISB3

#define can_bus_res_tris	TRISDbits.TRISD3
#define can_bus_res			PORTDbits.PORTD3


/** L E D           ********************************/
#define led_st1_tris		TRISDbits.TRISD0
#define led_st1				LATDbits.LATD0

#define led_st2_tris		TRISDbits.TRISD1
#define led_st2				LATDbits.LATD1

#define led_st3_tris 		TRISDbits.TRISD2
#define led_st3				LATDbits.LATD2


/** B U T T O N     ********************************/
#define button_sw2_tris		TRISBbits.TRISB0
#define button_sw2			PORTBbits.PORTB0


/** S W I T C H     ********************************/
#define switch_sw1_tris 	TRISCbits.TRISC0
#define switch_sw1			PORTCbits.PORTC0

#define switch_sw2_tris 	TRISCbits.TRISC1
#define switch_sw2			PORTCbits.PORTC1

#define switch_sw3_tris 	TRISCbits.TRISC2
#define switch_sw3			PORTCbits.PORTC2

#define switch_sw4_tris 	TRISCbits.TRISC3
#define switch_sw4			PORTCbits.PORTC3

#define switch_sw5_tris 	TRISCbits.TRISC4
#define switch_sw5			PORTCbits.PORTC4

#define switch_sw6_tris 	TRISCbits.TRISC5
#define switch_sw6			PORTCbits.PORTC5

#define switch_sw7_tris 	TRISEbits.TRISE0
#define switch_sw7			PORTEbits.PORTE0

#define switch_sw8_tris 	TRISEbits.TRISE1
#define switch_sw8			PORTEbits.PORTE1


#endif

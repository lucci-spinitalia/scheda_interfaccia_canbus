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
#include <p18cxxx.h>

/** O S C I L L A T O R  ***************************/
#define CRYSTAL_FREQUENCY_MHZ	16
#if 1//(CONFIG1Hbits.PLLCFG == 1)// && (CONFIG1Hbits.FOSC != 8) && (CONFIG1Hbits.FOSC != 9)
  #define FOSC_MHZ CRYSTAL_FREQUENCY_MHZ*4
#else
  #define FOSC_MHZ CRYSTAL_FREQUENCY_MHZ
#endif


/** T R I S         ********************************/
#define INPUT_PIN	1
#define OUTPUT_PIN 	0

/** U S A R T       ********************************/
#define uart1_tx_tris				TRISCbits.TRISC6
#define uart1_rx_tris				TRISCbits.TRISC7
#define uart1_rs485_tx_enable_tris	TRISDbits.TRISD4
#define uart1_rs485_tx_enable		LATDbits.LATD4

#define uart2_tx_tris				TRISDbits.TRISD6
#define uart2_rx_tris				TRISDbits.TRISD7
#define uart2_rs485_tx_enable_tris	TRISDbits.TRISD5
#define uart2_rs485_tx_enable		LATDbits.LATD5

/** C A N B U S     ********************************/
#define can_bus_tx_tris		TRISBbits.TRISB2

#define can_bus_rx_tris 	TRISBbits.TRISB3

#define can_bus_res_tris	TRISDbits.TRISD3
#define can_bus_res			PORTDbits.RD3


/** L E D           ********************************/
#define led_st1_tris		TRISDbits.TRISD0
#define led_st1				LATDbits.LATD0

#define led_st2_tris		TRISDbits.TRISD1
#define led_st2				LATDbits.LATD1

#define led_st3_tris 		TRISDbits.TRISD2
#define led_st3				LATDbits.LATD2


/** B U T T O N     ********************************/
#define button_sw2_tris		TRISBbits.TRISB0
#define button_sw2			PORTBbits.RB0


/** S W I T C H     ********************************/
#define switch_sw1_tris 	TRISCbits.TRISC0
#define switch_sw1			PORTCbits.RC0

#define switch_sw2_tris 	TRISCbits.TRISC1
#define switch_sw2			PORTCbits.RC1

#define switch_sw3_tris 	TRISCbits.TRISC2
#define switch_sw3			PORTCbits.RC2

#define switch_sw4_tris 	TRISCbits.TRISC3
#define switch_sw4			PORTCbits.RC3

#define switch_sw5_tris 	TRISCbits.TRISC4
#define switch_sw5			PORTCbits.RC4

#define switch_sw6_tris 	TRISCbits.TRISC5
#define switch_sw6			PORTCbits.RC5

#define switch_sw1_to_sw6	PORTC

#define switch_sw7_tris 	TRISEbits.TRISE0
#define switch_sw7			PORTEbits.RE0

#define switch_sw8_tris 	TRISEbits.TRISE1
#define switch_sw8			PORTEbits.RE1

#define switch_sw7_to_sw8	PORTE

/** M A X 3 1 6 0      *****************************/
#define max3160_shdn1_tris	TRISAbits.TRISA0
#define max3160_shdn1		LATAbits.LATA0

#define max3160_fast1_tris	TRISAbits.TRISA1
#define max3160_fast1		LATAbits.LATA1

#define max3160_rs4851_tris	TRISAbits.TRISA2
#define max3160_rs4851		LATAbits.LATA2

#define max3160_full1_tris	TRISAbits.TRISA5
#define max3160_full1		LATAbits.LATA5

#define max3160_shdn2_tris	TRISBbits.TRISB5
#define max3160_shdn2		LATBbits.LATB5

#define max3160_fast2_tris	TRISBbits.TRISB1
#define max3160_fast2		LATBbits.LATB1

#define max3160_rs4852_tris	TRISBbits.TRISB4
#define max3160_rs4852		LATBbits.LATB4

#define max3160_full2_tris	TRISEbits.TRISE2
#define max3160_full2		LATEbits.LATE2

/** M A X 3 1 6 0  M A C R O ***********************/
#define MAX3160_ENABLE1()	(max3160_shdn1 = 1)
#define MAX3160_ENABLE2()	(max3160_shdn2 = 1)
#define MAX3160_DISABLE1()	(max3160_shdn1 = 0)
#define MAX3160_DISABLE2()	(max3160_shdn2 = 0)

#define MAX3160_SLEW_CRTL_OFF1()	(max3160_fast1 = 1)
#define MAX3160_SLEW_CRTL_OFF2()	(max3160_fast2 = 1)
#define MAX3160_SLEW_CRTL_ON1()		(max3160_fast1 = 0)
#define MAX3160_SLEW_CRTL_ON2()		(max3160_fast2 = 0)

#define MAX3160_ENABLE_RS485_1()	{max3160_rs4851 = 1; max3160_full1 = 1;}
#define MAX3160_ENABLE_RS485_2()	{max3160_rs4852 = 1; max3160_full2 = 1;}
#define MAX3160_ENABLE_RS422_1()	{max3160_rs4851 = 1; max3160_full1 = 0; uart1_rs485_tx_enable = 1;}
#define MAX3160_ENABLE_RS422_2()	{max3160_rs4852 = 1; max3160_full2 = 0; uart2_rs485_tx_enable = 1;}
#define MAX3160_ENABLE_RS232_1()	{max3160_rs4851 = 0; max3160_full1 = 0;}
#define MAX3160_ENABLE_RS232_2()	{max3160_rs4852 = 0; max3160_full2 = 0;}

#define MAX3160_FULL_DUPLEX1()		(max3160_full1 = 0)
#define MAX3160_FULL_DUPLEX2()		(max3160_full2 = 0)
#define MAX3160_HALF_DUPLEX1()		(max3160_full1 = 1)
#define MAX3160_HALF_DUPLEX2()		(max3160_full2 = 1)

/** M C P 2 5 5 1  *********************************/
#define mcp2551_rs_tris
#define mcp2551_rs

#define can_terminator_tris		TRISDbits.TRISD3
#define can_terminator			LATDbits.LATD3

/** M C P 2 5 5 1  M A C R O ***********************/
#define MCP2551_TERMINATOR_ENABLE()	(can_terminator = 1)
#define MCP2551_TERMINATOR_DISABLE()(can_terminator = 0)

#endif // _IO_CFG_H

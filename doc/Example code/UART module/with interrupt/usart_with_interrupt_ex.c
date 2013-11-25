/***************************************************
* FileName:        main.c
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
* This example make an echo on uart2 using uart module with interrupt
*
* Note:
* A new micro's feauture is the internal regulator. The internal logic runs at 
* 3.3V, so, since our Vcc line is at 5V, we have to enable the internal 
* regulator by place a 10 uF capacitor with low ESR. It's possible to set the
* internal regulator to Low power or Ultra-low power mode (see cap. 28.3.3)
*
****************************************************/

/** I N C L U D E S ********************************/
#include "main.h"
#include <p18cxxx.h>
#include "system\io_cfg.h"
#include "user\can_to_rs232_converter.h"
#include "user\uart_interface.h"	// for test only. remove
unsigned char *temp;		// for test only. remove
/** C O N F I G ************************************/

#pragma config XINST 		= OFF // Legacy mode
#pragma config SOSCSEL  	= DIG // Secondary oscillator disable; RC0 and RC1 as
								// I/O pins
#pragma config INTOSCSEL	= HIGH // LF-INTOSC in High-Power mode during Sleep
#pragma config PWRTEN		= OFF // Ultra-low power mode disable	
#pragma config IESO			= OFF // Two-Speed Start-up is disabled
#pragma config FCMEN		= OFF // Fail-Safe Clock Monitor is disabled
#pragma config PLLCFG		= ON // Oscillator is multiplied by 4
#pragma config FOSC			= HS2 // HS oscillator (high power, 16 MHz-25 MHz)
#pragma config BORPWR		= LOW // BORMV is set to a low-power level; I don't 
								// need too much precision
#pragma config BORV			= 3 // BVDDis set to 1.8V
#pragma config BOREN		= SBORDIS // Brown-out Reset is enabled in hardware only
#pragma config WDTPS		= 8 // Watchdog Timer Postscale; 1:256 (1.024s)
#pragma config WDTEN		= SWDTDIS // WDT is enabled in hardware; SWDTEN bit is 
								// disabled
#pragma config MCLRE		= ON // MCLR  pin is enabled; RE3 input pin is 
								// disabled
#pragma config CANMX		= PORTB // CANTX and CANRX pins are located on RB2 and 
								// RB3, respectively
#pragma config BBSIZ		= BB1K // Boot Block Size - 1 kW boot block size
#pragma config STVREN		= ON // Stack Full/Underflow Reset Enable bit
#pragma config CP3			= OFF // Block 3 is not code-protected
#pragma config CP2			= OFF // Block 2 is not code-protected
#pragma config CP1			= OFF // Block 1 is not code-protected
#pragma config CP0			= OFF // Block 0 is not code-protected
#pragma config CPD			= OFF // Data EEPROM is not code-protected
#pragma config CPB			= OFF // Boot block is not code-protected
#pragma config WRT3			= OFF // Block 3 is not write-protected
#pragma config WRT2			= OFF // Block 2 is not write-protected
#pragma config WRT1			= OFF // Block 1 is not write-protected
#pragma config WRT0			= OFF // Block 0 is not write-protected
#pragma config WRTD			= OFF // Data EEPROM is not write-protected
#pragma config WRTB			= OFF // Boot block is not write-protected
#pragma config WRTC			= OFF // Configuration registers are not write-protected
#pragma config EBTR3		= OFF // Block 3 is not protected from table reads executed in other blocks
#pragma config EBTR2		= OFF // Block 2 is not protected from table reads executed in other blocks
#pragma config EBTR1		= OFF // Block 1 is not protected from table reads executed in other blocks
#pragma config EBTR0		= OFF // Block 0 is not protected from table reads executed in other blocks
#pragma config EBTRB		= OFF // Boot block is not protected from table reads executed in other blocks

#pragma code

/**************************************************
* Function name		: void main(void)
*
* Created by		: Luca Lucci
* Date created		: 07/10/12
* Description		: Main program entry point
* Notes				: -
**************************************************/
void main(void)
{
  initialize_system();

    // initialize usart
  uart_init();
  uart2_open();

  // initialize interrupt
  INTCON2bits.INTEDG0 = 0;	// falling edge int0 interrupt
  INTCONbits.INT0IE = 1;	// enable int0 interrupt
  INTCONbits.INT0IF = 0;
  INTCONbits.PEIE = 1;		// enable peripheral interrupts
  INTCONbits.GIE = 1;		// enable global interrupt
  

  while(1) 
  {
    ClrWdt();
    
    // if the buffer isn't empty will send back the data
    if(uart2_buffer_read(temp))
      uart2_buffer_tx_load(temp);

    // ProcessIO
  }
}//main()


/**************************************************
* Function name		: void initialize_system(void)
*
* Created by		: Luca Lucci
* Date created		: 18/10/12
* Description		: system based initializations: i/o pin,leds, switchs, 
*					  buttons
* Notes				: running this fuction will set all ports as digital one. 
*                     These ports will be set as output, but onboard buttons
*                     and switches. All output pins will be driven to low logic 
*                     state.
**************************************************/
void initialize_system(void)
{
  // initialize pins
  ANCON0 = 0;	// all port as digital
  ANCON1 = 0; 	// all port as digital

  /* Datasheet pag. 49: "Unused I/O pins should be configured as outputs and
   * driven to a logic low state. Alternatively, connect 1 kohm to 10 kohm
   * resistor to Vss on unused pins and drive the output to logic low */
  TRISA = OUTPUT_PIN;
  TRISB = OUTPUT_PIN;
  TRISC = OUTPUT_PIN;
  TRISD = OUTPUT_PIN;
  TRISE = OUTPUT_PIN;

  TRISA = 0;
  TRISB = 0;
  TRISC = 0;
  TRISD = 0;
  TRISE = 0;

 // initialize led,button,switch
  led_st1_tris = OUTPUT_PIN;
  led_st2_tris = OUTPUT_PIN;
  led_st3_tris = OUTPUT_PIN;

  led_st1 = 0;
  led_st2 = 0;
  led_st3 = 0;

  button_sw2_tris = INPUT_PIN;

  switch_sw1_tris = INPUT_PIN;
  switch_sw2_tris = INPUT_PIN;
  switch_sw3_tris = INPUT_PIN;
  switch_sw4_tris = INPUT_PIN;
  switch_sw5_tris = INPUT_PIN;
  switch_sw6_tris = INPUT_PIN;
  switch_sw7_tris = INPUT_PIN;
  switch_sw8_tris = INPUT_PIN;
}


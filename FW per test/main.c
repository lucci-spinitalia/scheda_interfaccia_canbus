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
* First written on 09/11/2012 by Luca Lucci.
*
* Module Description:
* This is the main module where initiliaze all peripherals and run the main
* loop. The software's purpouse is send a predefine set of characters with
* a predetermined delay. It's intended to be used to test the performance of 
* "Scheda interfaccia CANBUS" firmware.
*
****************************************************/

/** I N C L U D E S ********************************/
#include <p18cxxx.h>
#include "main.h"
#include "system/io_cfg.h"
#include "user/global.h"
#include "user/uart_interface.h"

/** C O N F I G ************************************/
#pragma config OSC    = HSPLL	// Oscillator Selection bits
#pragma config FCMEN  = OFF		// Fail-Safe clock monitor enable bit
#pragma config IESO   = OFF		// Internal/External oscillator switchover bit
#pragma config PWRT   = OFF 	// Power-up Timer Enable bit
#pragma config BOREN  = OFF		// Brown-out Voltage bits
#pragma config BORV   = 0		// Brown-out Voltage bits - 0: Maximum settig; 1; 2; 3: Minimum setting
#pragma config WDT    = OFF		// Watchdog Timer
#pragma config WDTPS  = 32768	// Watchdog Timer Postscale Select bits
#pragma config MODE   = MC		// Processor Data Memory Mode Select bits  - EM: Extended Microcontroller mode; MPB: Microprocessor with boot block mode; MP: Microprocessor mode; MC: Microcontroller mode
#pragma config ADDRBW = ADDR8BIT // Address Bus Width Select bits
#pragma config DATABW = DATA8BIT // Data Bus Width Select bit
#pragma config WAIT   = OFF		// External Bus Data Wait Enable bit
#pragma config MCLRE  = ON		// MCLR Pin Enable bit
#pragma config LPT1OSC = OFF	// Low-Power Timer1 Oscillator Enable bit
#pragma config ECCPMX = PORTH	// ECCP MUX bit
#pragma config CCP2MX = PORTBE	// CCP2 MUX bit - PORTBE; PORTC
#pragma config STVREN = OFF		// Stack Full/Underflow Reset Enable bit
#pragma config LVP    = OFF		// Single-Supply ICSP Enable bit
#pragma config BBSIZ  = BB2K    // BB2K: 1K word (2kbytes) boot block size
#pragma config XINST  = OFF		// Extended Instruction Set Enable bit
#pragma config CP0      = OFF
#pragma config CP1      = OFF
#pragma config CP2      = OFF
#pragma config CP3      = OFF
#pragma config CP4      = OFF
#pragma config CP5      = OFF
#pragma config CP6      = OFF
#pragma config CP7      = OFF
#pragma config CPB      = OFF
#pragma config CPD      = OFF
#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
#pragma config WRT2     = OFF
#pragma config WRT3     = OFF
#pragma config WRT4     = OFF
#pragma config WRT5     = OFF
#pragma config WRT6     = OFF
#pragma config WRT7     = OFF
#pragma config WRTB     = OFF       // Boot Block Write Protection
#pragma config WRTC     = OFF
#pragma config WRTD     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
#pragma config EBTR2    = OFF
#pragma config EBTR3    = OFF
#pragma config EBTR4    = OFF
#pragma config EBTR5    = OFF
#pragma config EBTR6    = OFF
#pragma config EBTR7    = OFF
#pragma config EBTRB    = OFF

#pragma code

/** V A R I A B L E S ********************************************************/
unsigned char data[50];

unsigned int size_data;
unsigned int num_can_message;
unsigned int timer_preload;

/**************************************************
* Function name		: void main(void)
*
* Created by		: Luca Lucci
* Date created		: 09/11/12
* Description		: Main program entry point
* Notes				: -
**************************************************/
void main(void)
{
  unsigned char i;

  initialize_system();

  num_can_message = 100;
  size_data = 1;

  for(i = 0; i < size_data; i++)
  	data[i] = i;

  tris_LED_D0 = 0;
  led_d0 = 0;

  tris_LED_D1 = 0;
  led_d1 = 0;

  tris_LED_D2 = 0;
  led_d2 = 0;

  tris_LED_D3 = 0;
  led_d3 = 0;

  tris_LED_D4 = 0;
  led_d4 = 0;

  tris_LED_D5 = 0;
  led_d5 = 0;

  tris_LED_D6= 0;
  led_d6 = 0;

  tris_LED_D7 = 0;
  led_d7 = 0; 

  // initialize interrupt flag
  interrupt_tris = OUTPUT_PIN;
  interrupt_flag = 0;

  // initialize interrupt
  T0CONbits.T0CS = 0;	//timer 0 as timer
  T0CONbits.T08BIT = 0;	//16 bit timer
  INTCONbits.TMR0IF = 0;	//clear int flag
  INTCONbits.TMR0IE = 0;

  T0CONbits.PSA = 0;
  T0CONbits.T0PS = 3;
  // For 40 MHz clock one tmr bit is 1/10 = 0.1 us. So, tmr it must be 
  // load with a value equal to (2^16 - time_out)*0.1 us. Every write on 
  // tmr0 register make two cycle where tmr0 is inhibited, so it must be
  // taken in account in timeout value.

  // 1 ms 
  timer_preload = 0xFB1E;
  timer0_reload(timer_preload);

  T0CONbits.TMR0ON = 1;	//enable timer 0

  // initialize uart interface
  uart1_tx_tris = OUTPUT_PIN;
  uart1_rx_tris = INPUT_PIN;
  
  //baud_config = BAUD_16_BIT_RATE & BAUD_AUTO_OFF & BAUD_WAKEUP_OFF;
  //baud1USART(baud_config);
  //Open1USART( USART_TX_INT_ON & USART_RX_INT_ON & USART_BRGH_HIGH & USART_CONT_RX & USART_EIGHT_BIT & USART_ASYNCH_MODE, 86);
 
  uart1_init();
  uart1_open();

  INTCON2bits.INTEDG0 = 0;	// falling edge int0 interrupt
  INTCONbits.INT0IE = 1;	// enable int0 interrupt
  INTCONbits.INT0IF = 0;
  INTCONbits.PEIE = 1;		// enable peripheral interrupts
  INTCONbits.GIE = 1;		// enable global interrupt

  while(1)
  {
    
  }
}

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
  ADCON1 = 0x0F;      //Digital pins
  CMCON = 0x07;       //Digital pins

  // Initialize led
  tris_LED_D0 = OUTPUT_PIN;
  tris_LED_D1 = OUTPUT_PIN;
  tris_LED_D2 = OUTPUT_PIN;
  tris_LED_D3 = OUTPUT_PIN;
  tris_LED_D4 = OUTPUT_PIN;
  tris_LED_D5 = OUTPUT_PIN;
  tris_LED_D6 = OUTPUT_PIN;

  // Initialize button
  button_tris = INPUT_PIN;
}

void timer0_reload(unsigned int timer_preload)
{
  TMR0L = timer_preload;
  TMR0H = timer_preload >> 8;

}
/**************************************************
* FileName:        version.h
* Dependencies:    See INCLUDES section below
* Processor:       PIC18
* Compiler:        C18 3.43
* Company:         SpinItalia s.r.l.
*
* All Rights Reserved
*
* The information contained herein is confidential
* property of Company. The use, copying, transfer or 
* disclosure of such information is prohibited except
* by express written agreement with Company.
*
* 03/02/13 - Version 2.0
*		* Add function in uart_interface and can_interface to handle error message
*		* change how to pass sync param to can_init function
*		* change can_init_buadrate_set to pass the sync param directly
*		* Add can_buffer_tx_free_update to update can tx buffer status when
*		* the interrupts are disable
*		* Add function can_buffer_set_priority to set the priority of a message
*		* Change managment of can tx buffer to make it a circle one
*		* In uart_buffer_tx_seq_load change the write order
*		* Fix function can_buffer_load that didn't work with param -1
*		* Add managment of error interrupt in can_isr()
*       * Add uart message delivery in can_to_rs232_loop() if interrupts are 
*         disabled
* 		* Rewrite state machine for can_to_rs232_converter
*		* Add explosive gas sensor
*		* Change dip switch address
* 21/01/13 - Version 1.1
*       * Add rs485 managment into uart_interface functions
* 07/10/12 - Version 1.0
*		* Initial release
*		* Add sensor sass2300
*		* Add pc interface
**************************************************/

/* Abbreviation Table
*  uart == universal asychronous receiver transmitter
*  tx	== transmitter
*  rx	== recevier
*  cnt 	== count
*  ptr	== pointer
*  wr 	== write
*  rd 	== read
*  op   == operational mode
*/

#ifndef VERSION
#define VERSION "Version 2.0"
#endif //VERSION

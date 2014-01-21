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
* 21/01/2014 - Version 3.2
*       * Modify chempro100 settings: now baudrate is set to 38400
*       * Change led's meaning: led3 lid when there's can error or uart error
*       * and led1 change status one time per second.
*
* 27/11/13 - Version 3.1
*       * Add sass3100 device
*       * Add version info to aswer to info request
*
* 09/07/13 - Version 3.0
*       * Changed addresses. Now the sending address and receive address differ
*         by 0x100
*       * Added sass2300 commands to change baudrate
*       * Now the code to aswer to the info request is 0x1c
*
* 09/05/13 - Version 2.0
*       * Add message supported on sensor info request (internal message)
*       * Add declaretion on reset
*       * Add declaration as internal command with code 0x0c
*       * Add to trasparent_uart_state_machine a way to collect more data in
*         one shot by count variable
*       * Modify the way to communicate with the board by can bus. Now filter
*         two indacates an internal message
*       * Modify sensor address. Now the odd address is the board, also pair
*         pair address is the sensor
*		* Add function in uart_interface and can_interface to handle error message
*		* change how to pass sync param to can_init function
*		* change can_init_buadrate_set to pass the sync param directly
*		* Add can_buffer_tx_free_update to update can tx buffer status when
*		  the interrupts are disable
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
#define VERSION 32 //"Version 3.2"
#endif //VERSION

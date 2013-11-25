/***************************************************
* FileName:        spi_interface.h
* Dependencies:    See INCLUDES section below
* Processor:       PIC18
* Compiler:        C18 3.43
* Company:         SpinItalia s.r.l.
* 
* All Rights Reserved.
*
* The information contained herein is confidential 
* property of SpinItalia s.r.l. The user, copying, transfer or 
* disclosure of such information is prohibited except
* by express written agreement with SpinItalia s.r.l.
*
* First written on 18/02/2013 by Luca Lucci.
*
* Module Description:
* Define prototype and const for spi_interface.c
*
****************************************************/
#ifndef _SPI_INTERFACE_H
#define _SPI_INTERFACE_H

/** I N C L U D E S ********************************/


/** D E F I N E ************************************/
#define SPI_BUFFER_SIZE_TX 10
#define SPI_BUFFER_SIZE_RX 10

/** S T R U C T S **********************************/
extern struct spi_status_struct
{
  unsigned buffer_tx_full 			:1;	// tx buffer full
  unsigned buffer_tx_empty 			:1; // tx buffer empty
  unsigned buffer_rx_full			:1; // rx buffer full
  unsigned buffer_rx_empty			:1; // rx buffer empty
  unsigned buffer_rx_overflow		:1; // rx buffer overflow
};

/** M A C R O  *************************************/
#define chip_select_enable() {spi_cs = 0; Nop();}
#define chip_select_disable() {Nop(); spi_cs = 1;}

/** S H A R E D  V A R I A B L E S  ****************/
extern struct spi_status_struct spi_status;	// uart's status flag

/** P R O T O T Y P E S ****************************/
void spi_init(void);
unsigned char spi_buffer_tx_load(unsigned char data);
unsigned char spi_buffer_tx_seq_load(unsigned char *data, unsigned char length);
unsigned char spi_buffer_send(void);
unsigned char spi_buffer_rx_load(void);
unsigned char spi_buffer_read(unsigned char *data);
unsigned char spi_get_rx_data_size(void);

#endif
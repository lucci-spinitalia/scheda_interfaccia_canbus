/***************************************************
* FileName:        spi_interface.c
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
* This module provide functions for spi interface
*
****************************************************/
#include "spi_interface.h"
#include "io_cfg.h"

/** S T R U C T S **********************************/
struct spi_status_struct
{
  unsigned buffer_tx_full 			:1;	// tx buffer full
  unsigned buffer_tx_empty 			:1; // tx buffer empty
  unsigned buffer_rx_full			:1; // rx buffer full
  unsigned buffer_rx_empty			:1; // rx buffer empty
  unsigned buffer_rx_overflow		:1; // rx buffer overflow
};

/** L O C A L  V A R I A B L E S  ******************/
struct spi_status_struct spi_status;	// uart's status flag

unsigned char spi_buffer_tx[SPI_BUFFER_SIZE_TX];
unsigned char spi_buffer_rx[SPI_BUFFER_SIZE_RX];

unsigned int spi_buffer_tx_data_cnt;	// number of byte to transmit
unsigned int spi_buffer_tx_wr_ptr;	// write position in tx buffer
unsigned int spi_buffer_rx_data_cnt;	// number of byte received
unsigned int spi_buffer_rx_wr_ptr;	// write position in rx buffer

/**************************************************
* Function name		: void spi_init(void)
*
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function initialize spi peripheral and it need to
*					  be called before using any functions related
* Notes				: After call it, the related pin will be set as input or 
*					  output as stated in datasheet
* TODO				: * one should can choose the baud rate
*					  * implement interrupt
**************************************************/
void spi_init(void)
{
  // init pin
  spi_sdo_tris = 0;	// sdo as output
  spi_sck_tris = 0;	// sck as output
  spi_cs_tris = 0;  // chip select as output

  spi_cs = 1; // set the cs pin high

  // Initialize the status variables and circular buffer variables
  spi_status.buffer_tx_full = 0;
  spi_status.buffer_tx_empty = 1;

  spi_buffer_tx_data_cnt = 0;
  spi_buffer_tx_wr_ptr = 0;

  spi_status.buffer_rx_full = 0;
  spi_status.buffer_rx_empty = 1;
  spi_status.buffer_rx_overflow = 0;
  spi_buffer_rx_data_cnt = 0;
  spi_buffer_rx_wr_ptr = 0;

  // Initialize baud rate
  SSPCON1bits.SSPM = 2;	// fosc/64 = 1 MHz @ 64MHz

  SSPCON1bits.SSPEN = 1; // enable spi

}

/**************************************************
* Function name		: unsigned char spi_buffer_tx_load(unsigned char data)
*	return			: 0 = failure - the buffer is full
*					: 1 = success - buffer loaded
*	data    		: unsigned char pointer to the data to load in the tx buffer
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function puts the data into tx buffer. It place the
*                	  argument "data" in transmit buffer and updates the data
*  					  count and write pointer variables.
* Notes				: The interrupt will be disable until the end 
**************************************************/
unsigned char spi_buffer_tx_load(unsigned char data)
{
  // Check if the buffer is full; if not, add one byte of data
  if(spi_status.buffer_tx_full)
    return 0;

  spi_buffer_tx[spi_buffer_tx_wr_ptr] = data;

  spi_status.buffer_tx_empty = 0;
  spi_buffer_tx_data_cnt++;

  if(spi_buffer_tx_data_cnt == SPI_BUFFER_SIZE_TX)
    spi_status.buffer_tx_full = 1;

  spi_buffer_tx_wr_ptr++;	//point to the next location

  if(spi_buffer_tx_wr_ptr == SPI_BUFFER_SIZE_TX)
    spi_buffer_tx_wr_ptr = 0;


  return 1;
}

/**************************************************
* Function name		: unsigned char spi_buffer_tx_seq_load(unsigned char *data, unsigned char length)
*	return			: 0 = failure - the buffer is full
*					: 1 = success - buffer loaded
*	data			: pointer to the data to load in the tx buffer
*	length			: number of byte store in data_write
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function puts sequential the data into tx buffer. It
*					  place the argument "data" in transmit buffer and 
*					  updates the data count and write pointer variables.
* Notes				: The interrupt will be disable until the end 
**************************************************/
unsigned char spi_buffer_tx_seq_load(unsigned char *data, unsigned char length)
{
  unsigned char i; //for loop

  //enable communication
  chip_select_enable()

  for(i = 0; i < length; i++)
  {

    if(!spi_buffer_tx_load(data[i]))
    {
	  chip_select_disable();
	  return 0;
	}
  }

  chip_select_disable()
  return 1;
}

/**************************************************
* Function name		: unsigned char spi_buffer_send(void)
*
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function sends the data from transmit buffer to 
*					  SPI and updates the data count and read pointer 
*					  variables of transmit buffer
* Notes				: if interrupt is disable, this is a blocking function!
*					  It wait until bus frees. Here there's the control for the
*					  rs485 buffer, so I don't bother if I use interrupt or
*					  polling.
**************************************************/
unsigned char spi_buffer_send(void)
{
  static unsigned int spi_buffer_tx_rd_ptr = 0;	// read position to place data
  if(!spi_status.buffer_tx_empty)
  {
    SSPBUF = spi_buffer_tx[spi_buffer_tx_rd_ptr];

    // check if there's some collision
    if(SSPCON1bits.WCOL)
    {
	  SSPCON1bits.WCOL = 0;
      return 0;
    }

    if(spi_status.buffer_tx_full)
      spi_status.buffer_tx_full = 0;

    spi_buffer_tx_data_cnt--;

    if(spi_buffer_tx_data_cnt == 0)
      spi_status.buffer_tx_empty = 1;

    spi_buffer_tx_rd_ptr++;

    if(spi_buffer_tx_rd_ptr == SPI_BUFFER_SIZE_TX)
      spi_buffer_tx_rd_ptr = 0;
  }

  return 1;
}

/**************************************************
* Function name		: unsigned char spi_get_tx_buffer_empty_space(void)
*	return			: 0 = no space left
*					  unsigned char = number of free byte in tx buffer
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function returns the number of bytes of free space 
*					  left out in transmit buffer at the calling time. It 
*					  helps the user to further write data into trasmit buffer
*					  at once, rather than checking transmit buffer.
* Notes				: -
**************************************************/
unsigned int spi_get_tx_buffer_empty_space(void)
{
  if(spi_buffer_tx_data_cnt < SPI_BUFFER_SIZE_TX)
    return(SPI_BUFFER_SIZE_TX - spi_buffer_tx_data_cnt);
  else
    return 0;
}

/**************************************************
* Function name		: unsigned char SPI_buffer_rx_load(void)
*	return			: 0 = failure - nothing to load
*					  1 = usart data loaded in rx buffer
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function load the data from the spi receiver to 
*					  the read buffer. It also check error states.
* Notes				: 
**************************************************/
unsigned char spi_buffer_rx_load(void)
{
  unsigned char chTemp;	// temporary variable for receiver

  if(!SSPSTATbits.BF)	// if there's no data return
    return 0;
  
  // If the device is in slave mode, I have to check for overflow
  // condition
  if(!SSPSTATbits.SMP && SSPCON1bits.SSPOV)
  {
    spi_status.buffer_rx_overflow = 1;

    chTemp = SSPBUF;

    // clear overflow condition
    SSPCON1bits.SSPOV = 0;
  }

  if(!spi_status.buffer_rx_full)
  {
    chTemp = SSPBUF;

    spi_status.buffer_rx_overflow = 0;
    spi_status.buffer_rx_empty = 0;
    spi_buffer_rx[spi_buffer_rx_wr_ptr] = chTemp;
    spi_buffer_rx_data_cnt++;
      
    if(spi_buffer_rx_data_cnt == SPI_BUFFER_SIZE_RX)
      spi_status.buffer_rx_full = 1;

    spi_buffer_rx_wr_ptr++;

    if(spi_buffer_rx_wr_ptr == SPI_BUFFER_SIZE_RX)
      spi_buffer_rx_wr_ptr = 0;
  }
  else
    return 0;

  return 1;
}

/**************************************************
* Function name		: unsigned char spi_buffer_read(unsigned char *data)
*	return			: 0 = buffer empty
*					  1 = data read correctly
*	data    		: unsigned char pointer where store the data from rx buffer
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function reads the data from the receive buffer. It
*					  places the data in to argument and updates the data count
*					  and read pointer
* Notes				: In this function the interrupt will be disabled, so it 
*					  can possible keeps the access pointer values proper.
**************************************************/
unsigned char spi_buffer_read(unsigned char *data)
{
  // read position by the application
  static unsigned int spi_buffer_rx_rd_ptr = 0;	

  if(spi_status.buffer_rx_empty)
    return 0;
  
  spi_status.buffer_rx_full = 0;

  *data = spi_buffer_rx[spi_buffer_rx_rd_ptr];
  
  spi_buffer_rx_data_cnt--;

  if(spi_buffer_rx_data_cnt == 0)
    spi_status.buffer_rx_empty = 1;

  spi_buffer_rx_rd_ptr++;

  if(spi_buffer_rx_rd_ptr == SPI_BUFFER_SIZE_RX)
    spi_buffer_rx_rd_ptr = 0;

  return 1;
}

/**************************************************
* Function name		: unsigned char spi_get_rx_data_size(void);
*	return			: number of byte to read through spi_buffer_rx_load
* Created by		: Luca Lucci
* Date created		: 18/02/13
* Description		: This function returns the number of bytes of data 
*					  available in receive buffer at the calling time. It helps
					  the user to read data from receive buffer at once.
* Notes				: -
**************************************************/
unsigned char spi_get_rx_data_size(void)
{
  return spi_buffer_rx_data_cnt;
}
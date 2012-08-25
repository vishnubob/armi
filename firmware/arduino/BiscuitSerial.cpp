#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"
#include "wiring_private.h"
#include "BiscuitSerial.h"

#define SERIAL_BUFFER_SIZE 16

struct ring_buffer
{
  unsigned char buffer[SERIAL_BUFFER_SIZE];
  volatile int head;
  volatile int tail;
};

ring_buffer rx_buffer = { { 0 }, 0, 0};
ring_buffer tx_buffer = { { 0 }, 0, 0};
ring_buffer rx_meta_buffer = { { 0 }, 0, 0};

inline void store_char(unsigned char c, ring_buffer *buffer)
{
  int i = (unsigned int)(buffer->head + 1) % SERIAL_BUFFER_SIZE;

  // if we should be storing the received character into the location
  // just before the tail (meaning that the head would advance to the
  // current location of the tail), we're about to overflow the buffer
  // and so we don't write the character or advance the head.
  if (i != buffer->tail) {
    buffer->buffer[buffer->head] = c;
    buffer->head = i;
  }
}

ISR(USART_RX_vect)
{
    unsigned char status = UCSR0A;
    unsigned char ch = UCSR0B;
    unsigned char c = UDR0;
    unsigned char meta = 0;
    if (status & FE0)
        sbi(meta, FRAME_ERROR);
    if (status & UPE0)
        sbi(meta, PARITY_ERROR);
    if (status & DOR0)
        sbi(meta, OVERRUN_ERROR);
    /*
    if (ch & RXB80)
        sbi(meta, NINTH_BIT_SET);
    */

    store_char(meta, &rx_meta_buffer);
    store_char(c, &rx_buffer);
}

ISR(USART_UDRE_vect)
{
    if (tx_buffer.head == tx_buffer.tail) 
    {
        cbi(UCSR0B, UDRIE0);
    } else 
    {
        // There is more data in the output buffer. Send the next byte
        unsigned char c = tx_buffer.buffer[tx_buffer.tail];
        tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
        UDR0 = c;
    }
}

// Public Methods //////////////////////////////////////////////////////////////

void BiscuitSerial::begin(unsigned long baud)
{
  uint16_t baud_setting;
  bool use_u2x = true;

try_again:
  
  if (use_u2x) {
    UCSR0A = 1 << U2X0;
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  } else {
    UCSR0A = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }
  
  if ((baud_setting > 4095) && use_u2x)
  {
    use_u2x = false;
    goto try_again;
  }

  // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
  UBRR0H = baud_setting >> 8;
  UBRR0L = baud_setting;

  sbi(UCSR0B, RXEN0);
  sbi(UCSR0B, TXEN0);
  sbi(UCSR0B, RXCIE0);  
  cbi(UCSR0B, UDRIE0);

  // setup 9bit
  /*
  sbi(UCSR0C, UCSZ00);
  sbi(UCSR0C, UCSZ01);
  sbi(UCSR0B, UCSZ02);
  */
  // setup even parity
  //sbi(UCSR0C, UPM01);
}

void BiscuitSerial::end()
{
  // wait for transmission of outgoing data
  while (tx_buffer.head != tx_buffer.tail)
    ;

  cbi(UCSR0B, RXEN0);
  cbi(UCSR0B, TXEN0);
  cbi(UCSR0B, RXCIE0);  
  cbi(UCSR0B, UDRIE0);
  
  // clear any received data
  rx_buffer.head = rx_buffer.tail;
}

int BiscuitSerial::available(void)
{
  return (unsigned int)(SERIAL_BUFFER_SIZE + rx_buffer.head - rx_buffer.tail) % SERIAL_BUFFER_SIZE;
}

int BiscuitSerial::peek(void)
{
  if (rx_buffer.head == rx_buffer.tail) {
    return -1;
  } else {
    return rx_buffer.buffer[rx_buffer.tail];
  }
}

int BiscuitSerial::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (rx_buffer.head == rx_buffer.tail) {
    return -1;
  } else {
    unsigned char c = rx_buffer.buffer[rx_buffer.tail];
    rx_buffer.tail = (unsigned int)(rx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
    unsigned char ch = rx_meta_buffer.buffer[rx_meta_buffer.tail];
    rx_meta_buffer.tail = (unsigned int)(rx_meta_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
    return (ch << 8) | c;
  }
}

void BiscuitSerial::flush()
{
  while (tx_buffer.head != tx_buffer.tail)
    ;
}

size_t BiscuitSerial::write(uint8_t c)
{
  int i = (tx_buffer.head + 1) % SERIAL_BUFFER_SIZE;
	
  // If the output buffer is full, there's nothing for it other than to 
  // wait for the interrupt handler to empty it a bit
  // ???: return 0 here instead?
  while (i == tx_buffer.tail)
    ;
	
  tx_buffer.buffer[tx_buffer.head] = c;
  tx_buffer.head = i;
	
  sbi(UCSR0B, UDRIE0);
  
  return 1;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

BiscuitSerial Serial;

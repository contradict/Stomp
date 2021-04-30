#include "Arduino.h"
#include "DMASerial.h"
#include "wiring_private.h"
#include <util/atomic.h>

void DMASerial::_tx_udr_empty_irq(void)
{
  // If interrupts are enabled, there must be more data in the output
  // buffer. Send the next byte
  size_t xferred = Chunks[chunk_tail].transferred;
  // cts_state is actibve LOW
  if(!cts_enabled || (cts_enabled && !cts_state))
  {
      *_udr = Chunks[chunk_tail].begin[xferred++];
      Chunks[chunk_tail].transferred = xferred;
  } else {
      cbi(*_ucsrb, UDRIE0);
      return;
  }

  // clear the TXC bit -- "can be cleared by writing a one to its bit
  // location". This makes sure flush() won't return until the bytes
  // actually got written
  sbi(*_ucsra, TXC0);

  if(xferred == Chunks[chunk_tail].length)
  {
      size_t new_tail = (chunk_tail + 1) % MAX_CHUNKS;
      if(new_tail == chunk_head)
      {
          // Buffer empty, so disable interrupts
          cbi(*_ucsrb, UDRIE0);
      }
      if(Chunks[chunk_tail].complete)
      {
          Chunks[chunk_tail].complete(
              Chunks[chunk_tail].funcdata,
              Chunks[chunk_tail].begin,
              Chunks[chunk_tail].length);
      }
      chunk_tail = new_tail;
  }
}

void DMASerial::set_cts_pin(uint8_t pin)
{
    if(pin > 0)
    {
        cts_pin = pin;
        cts_state = digitalRead(cts_pin);
        cts_enabled = true;
    } else {
        cts_enabled = false;
    }
}

void DMASerial::cts_interrupt()
{
    if(!cts_enabled)
        return;
    bool new_cts = digitalRead(cts_pin);
    bool falling = cts_state && !new_cts;
    cts_state = new_cts;
    if(falling && (chunk_head != chunk_tail) && bit_is_clear(*_ucsrb, UDRIE0))
    {
        sbi(*_ucsrb, UDRIE0);
    }
}

bool DMASerial::enqueue(const unsigned char *begin, size_t length, void *funcdata,
                        transfer_complete_func complete)
{
    _written = true;
    size_t next_spot = (chunk_head+1)%MAX_CHUNKS;
    while (next_spot == chunk_tail) {
        if(cts_enabled)
        {
            cts_state = digitalRead(cts_pin);
            // cts_state high means don't send
            if(cts_state)
            {
                return false;
            }
        }
        if (bit_is_clear(SREG, SREG_I)) {
            // Interrupts are disabled, so we'll have to poll the data
            // register empty flag ourselves. If it is set, pretend an
            // interrupt has happened and call the handler to free up
            // space for us.
            if(bit_is_set(*_ucsra, UDRE0))
                _tx_udr_empty_irq();
        } else {
            // nop, the interrupt handler will free up space for us
        }
    }
    Chunks[chunk_head].begin = begin;
    Chunks[chunk_head].length = length;
    Chunks[chunk_head].transferred = 0;
    Chunks[chunk_head].funcdata = funcdata;
    Chunks[chunk_head].complete = complete;
    chunk_head = next_spot;
    if(bit_is_clear(*_ucsrb, UDRIE0))
    {
        sbi(*_ucsrb, UDRIE0);
    }
    return true;
}

void DMASerial::advance_buffer_tail(const unsigned char *end)
{
    _tx_buffer_tail = end - &_tx_buffer[0];
}

void _advance_buffer_tail(void *funcdata, const unsigned char * begin, size_t length)
{
    ((DMASerial*)funcdata)->advance_buffer_tail(begin+length);
}

size_t DMASerial::write(const uint8_t *buffer, size_t size)
{
    // If the output buffer is full, there's nothing for it other than to
    // wait for the interrupt handler to empty it a bit
    while ((size_t)availableForWrite()<min(size, (size_t)(SERIAL_TX_BUFFER_SIZE-1))) {
        if(cts_enabled)
        {
            cts_state = digitalRead(cts_pin);
            // cts_state high means don't send
            if(cts_state)
            {
                return 0;
            }
        }
        if (bit_is_clear(SREG, SREG_I)) {
            // Interrupts are disabled, so we'll have to poll the data
            // register empty flag ourselves. If it is set, pretend an
            // interrupt has happened and call the handler to free up
            // space for us.
            if(bit_is_set(*_ucsra, UDRE0))
                _tx_udr_empty_irq();
        } else {
            // nop, the interrupt handler will free up space for us
        }
    }

    size_t copied;
    size_t head, tail;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        head = _tx_buffer_head;
        tail = _tx_buffer_tail;
    }
    if(tail>head)
    {
        copied = min(size, (size_t)(tail-head-1));
        memcpy(&_tx_buffer[head], buffer, copied);
    } else {
        copied = min(size, (size_t)(SERIAL_TX_BUFFER_SIZE-head));
        memcpy(&_tx_buffer[head], buffer, copied);
    }
    size_t new_head = (head+copied)%SERIAL_TX_BUFFER_SIZE;
    if(!enqueue(&_tx_buffer[head], copied, this, _advance_buffer_tail))
    {
        return 0;
    }
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        _tx_buffer_head = new_head;
    }
    if(copied<size) {
        copied += write(buffer+copied, size-copied);
    }
    return copied;
}

size_t DMASerial::write(uint8_t c)
{
  return write(&c, 1);
}

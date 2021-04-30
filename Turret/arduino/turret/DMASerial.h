#include <stdint.h>
#include <HardwareSerial.h>

class DMASerial : public HardwareSerial {
    volatile bool cts_state;
    bool cts_enabled;
    uint8_t cts_pin;
    volatile uint8_t chunk_head, chunk_tail;
    static const size_t MAX_CHUNKS=32;
    typedef void (*transfer_complete_func)(void * funcdata,
                                           const unsigned char *begin,
                                           size_t length);
    struct Chunk {
        const unsigned char *begin;
        size_t length;
        size_t transferred;
        void *funcdata;
        transfer_complete_func complete;
    } Chunks[MAX_CHUNKS];
    public:
    inline DMASerial(
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr);
    size_t write(const uint8_t *buffer, size_t size);
    size_t write(uint8_t c);
    bool enqueue(const unsigned char *begin, size_t length, void *funcdata,
                 transfer_complete_func complete);
    void set_cts_pin(uint8_t pin);
    void cts_interrupt();
    void _tx_udr_empty_irq(void);
    void advance_buffer_tail(const unsigned char *end);
};

extern DMASerial DSerial;

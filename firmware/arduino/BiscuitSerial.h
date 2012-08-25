#ifndef BiscuitSerial_h
#define BiscuitSerial_h

#include <inttypes.h>

#include "arduino/Stream.h"

struct ring_buffer;

#define NINTH_BIT_SET   0
#define PARITY_ERROR    1
#define OVERRUN_ERROR   2
#define FRAME_ERROR     4

class BiscuitSerial : public Stream
{
  public:
    void begin(unsigned long);
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

extern BiscuitSerial Serial;

#endif

#ifndef Slip_h
#define Slip_h


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

typedef unsigned char uchar;

//#include <stdlib.h>

#include <SoftwareSerial.h>

/* SLIP special character codes
 */
#define SLIP_END             0xC0    /* indicates end of packet */
#define SLIP_ESC             0xDB    /* indicates byte stuffing */
#define SLIP_ESC_END         0xDC    /* ESC ESC_END means END data byte */
#define SLIP_ESC_ESC         0xDE    /* ESC ESC_ESC means ESC data byte */

#define SLIP_MAXINBUFFERSIZE      255

class Slip
{
public:
    typedef void (*ReadCallback)(uint8_t *,uint8_t);
    
    Slip(SoftwareSerial& serial);
    ~Slip();
    void proc();
    bool sendpacket(uint8_t *buf,uint8_t len);
    void setCallback(ReadCallback cb)
    {
        readCallback = cb;
    }
private:

    ReadCallback readCallback = NULL;
    uint8_t lastbyte = 0;
    void handleByte(uint8_t c);
    uint8_t inbuffer[SLIP_MAXINBUFFERSIZE];
    uint8_t inbuffer_ndx=0;
    SoftwareSerial& _serial;
};


#endif
#ifndef Slip_h
#define Slip_h


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

typedef unsigned char uchar;

#include <SoftwareSerial.h>

/* SLIP special character codes
 */
#define SLIP_END             0xC0    /* indicates end of packet */
#define SLIP_ESC             0xDB    /* indicates byte stuffing */
#define SLIP_ESC_END         0xDC    /* ESC ESC_END means END data byte */
#define SLIP_ESC_ESC         0xDE    /* ESC ESC_ESC means ESC data byte */

#define SLIP_MAXINBUFFERSIZE      255

template<class TSer>
class Slip
{
public:
    typedef void (*ReadCallback)(uint8_t *,uint8_t);
    
    Slip(TSer& serial):_serial(serial){};
    ~Slip(){}
    void proc()
    {
        while (_serial.available() > 0)
        {
            handleByte(_serial.read());
        }
    };

    bool sendpacket(uint8_t *buf,uint8_t len)
    {
        _serial.write(SLIP_END);
        for(int i=0;i<len;i++)
        {
            switch(buf[i])
            {
            case SLIP_END:
                _serial.write(SLIP_ESC);
                _serial.write(SLIP_ESC_END);
                break;
            case SLIP_ESC:
                _serial.write(SLIP_ESC);
                _serial.write(SLIP_ESC_ESC);
                break;
            default:
                _serial.write(buf[i]);
            }
        }
        _serial.write(SLIP_END);
        return true;
    };

    void setCallback(ReadCallback cb)
    {
        readCallback = cb;
    };

private:
    void handleByte(uint8_t c)
    {

        if(c==SLIP_END)
        {
            if(inbuffer_ndx>0)
            {
                if(readCallback)
                {
                    readCallback(inbuffer,inbuffer_ndx);
                }
            }
            inbuffer_ndx=0;
            lastbyte=c;
            return;
        }

        if(inbuffer_ndx>=SLIP_MAXINBUFFERSIZE)inbuffer_ndx=0;

        if(lastbyte==SLIP_ESC)
        {
            switch(c)
            {
            case SLIP_ESC_END:
                inbuffer[inbuffer_ndx]=SLIP_END;
                inbuffer_ndx++;
                break;
            case SLIP_ESC_ESC:
                inbuffer[inbuffer_ndx]=SLIP_ESC;
                inbuffer_ndx++;
                break;
            }
            lastbyte=c;
            return;
        }

        if(c!=SLIP_ESC)
        {
            inbuffer[inbuffer_ndx]=c;
            inbuffer_ndx++;
        }

        lastbyte=c;
    };

    ReadCallback readCallback = NULL;
    uint8_t lastbyte = 0;
    uint8_t inbuffer[SLIP_MAXINBUFFERSIZE];
    uint8_t inbuffer_ndx=0;
    TSer& _serial;
};

typedef Slip<SoftwareSerial> SoftwareSlip;
typedef Slip<HardwareSerial> HardwareSlip;

#endif
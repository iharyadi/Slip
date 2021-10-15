#ifndef SLIP_H
#define SLIP_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

typedef unsigned char uchar;

#if F_CPU == 16000000 || F_CPU == 8000000 || F_CPU == 20000000
#include <SoftwareSerial.h>
#endif
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
#if !defined(DEVICE_SERIAL_ASYNCH)
    bool sendpacket(uint8_t *buf,uint8_t len)
    {
        _serial.write(SLIP_END);
        for(uint8_t i=0;i<len;i++)
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
#else
    bool sendpacket(uint8_t *buf,uint8_t len)
    {
        const uint8_t MAX_BUFFER = 64;
        uint8_t buffWrite[MAX_BUFFER+1];

        if(len > MAX_BUFFER)
        {
            return false;
        }

        if(len == 0)
        {
            return false;
        }

        buffWrite[0] = SLIP_END;
        uint8_t*  ndxBuff = &buffWrite[1];
        
        uint8_t i=0;

        for(;i<len && ndxBuff < &buffWrite[MAX_BUFFER];i++)
        {
            switch(buf[i])
            {
            case SLIP_END:
                *ndxBuff = SLIP_ESC;
                ndxBuff++;
                *ndxBuff = SLIP_ESC_END;
                break;
            case SLIP_ESC:
                *ndxBuff = SLIP_ESC;
                ndxBuff++;
                *ndxBuff =SLIP_ESC_ESC;
                break;
            default:
                *ndxBuff = buf[i];
            }
            ndxBuff++;
        }

        if(i < len && (ndxBuff-&buffWrite[0]) == MAX_BUFFER+1)
        {
            return false;
        }

        return _serial.write(buffWrite,(ndxBuff-&buffWrite[0])) == len;
    }
#endif

    void setCallback(ReadCallback cb)
    {
        readCallback = cb;
    };

    void begin()
    {
        _serial.begin(9600);
    }

private:
    void handleByte(uint8_t c)
    {
      if(inbuffer_ndx>=SLIP_MAXINBUFFERSIZE)
      {
        inbuffer_ndx=0;
        lastbyte = 0;
        return;
      } 
      
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
      } 
      else if(c==SLIP_END)
      {
        if(inbuffer_ndx>0)
        {
          if(readCallback)
          {
              readCallback(inbuffer,inbuffer_ndx);
          }
        }
        inbuffer_ndx=0;
      } 
      else if(c==SLIP_ESC)
      {
        lastbyte=c;
      }
      else
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

#if F_CPU == 16000000 || F_CPU == 8000000 || F_CPU == 20000000
    typedef Slip<SoftwareSerial> SoftwareSlip;
#endif

typedef Slip<HardwareSerial> HardwareSlip;

#endif
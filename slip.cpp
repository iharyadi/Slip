#include "Slip.h"

Slip::Slip(SoftwareSerial& serial):_serial(serial)
{

}

Slip::~Slip()
{

}

void Slip::handleByte(uint8_t c)
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
}

bool Slip::sendpacket(uint8_t *buf,uint8_t len)
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
}

void Slip::proc()
{
    while (_serial.available() > 0)
    {
        handleByte(_serial.read());
    }
}
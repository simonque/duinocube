/* Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com */
/* MAX3421E functions */
#ifndef _MAX3421E_H_
#define _MAX3421E_H_

#include <stdint.h>

#include "Max3421e_constants.h"

typedef uint8_t byte;
typedef uint8_t boolean;

#define LOW    0
#define HIGH   1

class MAX3421E /* : public SPI */ {
    // byte vbusState;
    public:
        MAX3421E( void );
        byte getVbusState( void );
        static void regWr( byte, byte );
        char * bytesWr( byte, byte, char * );
        static void gpioWr( byte );
        byte regRd( byte );
        char * bytesRd( byte, byte, char * );
        byte gpioRd( void );
        boolean reset();
        boolean vbusPwr ( boolean );
        void busprobe( void );
        void powerOn();
        byte IntHandler();
        byte GpxHandler();
        byte Task();
    private:
        byte poweredOn;
};

#endif //_MAX3421E_H_

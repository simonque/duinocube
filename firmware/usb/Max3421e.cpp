/* Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com */
/* MAX3421E USB host controller support */

#include <stdio.h>

#include <avr/io.h>

#include "defines.h"
#include "spi.h"

#include "Max3421e.h"

static byte vbusState;

#define MAX_PORT          PORTC     // Max3421e signals are on port C.
#define MAX_PORT_DDR      DDRC
#define MAX_RESET         PORTC2    // Reset (active-low).
#define MAX_GPX           PORTC1    // General purpose data from Max3421e.
#define MAX_INT           PORTC0    // Interrupt (active-low).

/* Functions    */

/* Constructor */
MAX3421E::MAX3421E()
{
  spi_init();

  // Set up the GPIO port used by Max3421e.
  MAX_PORT_DDR |= (1 << MAX_RESET);
  MAX_PORT_DDR &= ~((1 << MAX_GPX) | (1 << MAX_INT));

  // Reset the host.  Hold nRESET low for at least 200 ns.  At 20 MHz, that is
  // four clock cycles.
  MAX_PORT &= ~(1 << MAX_RESET);
  MAX_PORT &= ~(1 << MAX_RESET);
  MAX_PORT &= ~(1 << MAX_RESET);
  MAX_PORT &= ~(1 << MAX_RESET);
  MAX_PORT |= (1 << MAX_RESET);
}

byte MAX3421E::getVbusState( void )
{ 
    return( vbusState );
}
/* Single host register write   */
void MAX3421E::regWr( byte reg, byte val)
{
  spi_set_ss(DEV_SELECT_USB);
  spi_tx(reg | 0x02);
  spi_tx(val);
  spi_set_ss(DEV_SELECT_NONE);
  return;
}
/* multiple-byte write */
/* returns a pointer to a memory position after last written */
char * MAX3421E::bytesWr( byte reg, byte nbytes, char * data )
{
  spi_set_ss(DEV_SELECT_USB);
  spi_tx(reg | 0x02);
  while (nbytes--) {
    spi_tx(*data);                  // send next data byte
    data++;                         // advance data pointer
  }
  spi_set_ss(DEV_SELECT_NONE);
  return data;
}
/* GPIO write. GPIO byte is split between 2 registers, so two writes are needed to write one byte */
/* GPOUT bits are in the low nibble. 0-3 in IOPINS1, 4-7 in IOPINS2 */
/* upper 4 bits of IOPINS1, IOPINS2 are read-only, so no masking is necessary */
void MAX3421E::gpioWr( byte val )
{
    regWr( rIOPINS1, val );
    val = val >>4;
    regWr( rIOPINS2, val );
    
    return;     
}
/* Single host register read        */
byte MAX3421E::regRd( byte reg )    
{
  spi_set_ss(DEV_SELECT_USB);
  spi_tx(reg);
  uint8_t result = spi_tx(0);
  spi_set_ss(DEV_SELECT_NONE);

  return result;
}
/* multiple-bytes register read                             */
/* returns a pointer to a memory position after last read   */
char * MAX3421E::bytesRd ( byte reg, byte nbytes, char  * data )
{
  spi_set_ss(DEV_SELECT_USB);
  spi_tx(reg);
  while (nbytes--) {
    *data = spi_tx(0);       // Send empty byte.
    data++;
  }
  spi_set_ss(DEV_SELECT_NONE);
  return data;
}
/* GPIO read. See gpioWr for explanation */
/* GPIN pins are in high nibbles of IOPINS1, IOPINS2    */
byte MAX3421E::gpioRd( void )
{
 byte tmpbyte = 0;
    tmpbyte = regRd( rIOPINS2 );            //pins 4-7
    tmpbyte &= 0xf0;                        //clean lower nibble
    tmpbyte |= ( regRd( rIOPINS1 ) >>4 ) ;  //shift low bits and OR with upper from previous operation. Upper nibble zeroes during shift, at least with this compiler
    return( tmpbyte );
}
/* reset MAX3421E using chip reset bit. SPI configuration is not affected   */
boolean MAX3421E::reset()
{
  uint16_t tmp = 0;
    regWr( rUSBCTL, bmCHIPRES );                        //Chip reset. This stops the oscillator
    regWr( rUSBCTL, 0x00 );                             //Remove the reset
    while(!(regRd( rUSBIRQ ) & bmOSCOKIRQ )) {          //wait until the PLL is stable
        tmp++;                                          //timeout after 2^16 attempts
        if( tmp == 0 ) {
            return( false );
        }
    }
    return( true );
}
/* turn USB power on/off                                                */
/* does nothing, returns TRUE. Left for compatibility with old sketches               */
/* will be deleted eventually                                           */
///* ON pin of VBUS switch (MAX4793 or similar) is connected to GPOUT7    */
///* OVERLOAD pin of Vbus switch is connected to GPIN7                    */
///* OVERLOAD state low. NO OVERLOAD or VBUS OFF state high.              */
boolean MAX3421E::vbusPwr ( boolean action )
{
    return( true );                                             // power on/off successful                       
}
/* probe bus to determine device presense and speed and switch host to this speed */
void MAX3421E::busprobe( void )
{
 byte bus_sample;
    bus_sample = regRd( rHRSL );            //Get J,K status
    bus_sample &= ( bmJSTATUS|bmKSTATUS );      //zero the rest of the byte
    switch( bus_sample ) {                          //start full-speed or low-speed host 
        case( bmJSTATUS ):
            if(( regRd( rMODE ) & bmLOWSPEED ) == 0 ) {
                regWr( rMODE, MODE_FS_HOST );       //start full-speed host
                vbusState = FSHOST;
            }
            else {
                regWr( rMODE, MODE_LS_HOST);        //start low-speed host
                vbusState = LSHOST;
            }
            break;
        case( bmKSTATUS ):
            if(( regRd( rMODE ) & bmLOWSPEED ) == 0 ) {
                regWr( rMODE, MODE_LS_HOST );       //start low-speed host
                vbusState = LSHOST;
            }
            else {
                regWr( rMODE, MODE_FS_HOST );       //start full-speed host
                vbusState = FSHOST;
            }
            break;
        case( bmSE1 ):              //illegal state
            vbusState = SE1;
            break;
        case( bmSE0 ):              //disconnected state
            regWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ);
            vbusState = SE0;
            break;
        }//end switch( bus_sample )
}
/* MAX3421E initialization after power-on   */
void MAX3421E::powerOn()
{
    /* Configure full-duplex SPI, interrupt pulse   */
    regWr( rPINCTL,( bmFDUPSPI + bmINTLEVEL + bmGPXB ));    //Full-duplex SPI, level interrupt, GPX
    if( reset() == false ) {                                //stop/start the oscillator
        printf("Error: OSCOKIRQ failed to assert.\n");
    }

    /* configure host operation */
    regWr( rMODE, bmDPPULLDN|bmDMPULLDN|bmHOST|bmSEPIRQ );      // set pull-downs, Host, Separate GPIN IRQ on GPX
    regWr( rHIEN, bmCONDETIE|bmFRAMEIE );                                             //connection detection
    /* check if device is connected */
    regWr( rHCTL,bmSAMPLEBUS );                                             // sample USB bus
    while(!(regRd( rHCTL ) & bmSAMPLEBUS ));                                //wait for sample operation to finish
#ifdef DEBUG
    printf("Done sampling bus.\n");
#endif

    busprobe();                                                             //check if anything is connected
    regWr( rHIRQ, bmCONDETIRQ );                                            //clear connection detect interrupt                 
    regWr( rCPUCTL, 0x01 );                                                 //enable interrupt pin
}
/* MAX3421 state change task and interrupt handler */
byte MAX3421E::Task( void )
{
 byte rcode = 0;
 byte pinvalue;
    //Serial.print("Vbus state: ");
    //Serial.println( vbusState, HEX );
    pinvalue = (bool) (MAX_PORT & (1 << MAX_INT));
    if( pinvalue  == LOW ) {
        rcode = IntHandler();
    }
    pinvalue = (bool) (MAX_PORT & (1 << MAX_GPX));
    if( pinvalue == LOW ) {
        GpxHandler();
    }
    return( rcode );   
}   
byte MAX3421E::IntHandler()
{
 byte HIRQ;
 byte HIRQ_sendback = 0x00;
    HIRQ = regRd( rHIRQ );                  //determine interrupt source
    //if( HIRQ & bmFRAMEIRQ ) {               //->1ms SOF interrupt handler
    //    HIRQ_sendback |= bmFRAMEIRQ;
    //}//end FRAMEIRQ handling
    if( HIRQ & bmCONDETIRQ ) {
        busprobe();
        HIRQ_sendback |= bmCONDETIRQ;
    }
    /* End HIRQ interrupts handling, clear serviced IRQs    */
    regWr( rHIRQ, HIRQ_sendback );
    return( HIRQ_sendback );
}
byte MAX3421E::GpxHandler()
{
 byte GPINIRQ = regRd( rGPINIRQ );          //read GPIN IRQ register
    return( GPINIRQ );
}

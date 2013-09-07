// USBJoystick.h
//
// USB Interface to USB joystick/game pad, such as Logitech 'Dual Action' game pad
///
/// \mainpage Arduino USB Interface to USB joystick/game pad
///
/// This is the USBJoystick library.
/// It provides an Arduino library and class for reading input from a 
/// USB joystick/game pad, such as Logitech 'Dual Action' game pad.
/// It can be subclassed to get control when new input values are read, or when they actually change.
/// Alternatively, you can set up callback functions to be called 
/// when new input values are read, or when they actually change.
/// 
/// The version of the package that this documentation refers to can be downloaded 
/// from http://www.airspayce.com/mikem/arduino/USBJoystick/USBJoystick-1.2.zip
/// You can find the latest version at http://www.airspayce.com/mikem/arduino/USBJoystick
///
/// \par Prerequisites
///
/// Requires:
/// - SparkFun USB Host Shield
/// - USB_Host_Shield library. Install this in your libraries folder with a command like:
/// \code
/// git clone git://github.com/felis/USB_Host_Shield.git
/// \endcode
///
/// This library does not steal any timers or add any interrupt handlers.
/// 
/// \par Example programs
///
/// Example Arduino programs are included to show the main modes of use.
///
/// The following example programs are provided:
/// - usbjoystick_test: Simple demonstration that prints to Serial when input changes
///
/// \par Installation
///
/// Install in the usual way: unzip the distribution zip file to the libraries
/// sub-folder of your sketchbook. 
///
/// If you are using the older Sparkfun DEV-09628 USB Host Shield, you will need to alter the 
/// pin definitions in Max3421e_constants.h in the USB_Host_Shield library as discussed in 
/// http://www.sparkfun.com/products/9628
///
/// If you require the USB Host Shield to be installed with WiShield, expect pin conflicts. These can be resolved by 
/// Bending the USB Host Shield pins D8, D9 and D10 out of the way and then jumpering Host Shield D10 to D6,
/// Host Shield D9 to D3, Host Shield D8 to D5, then changing the settings in Max3421e_constants.h like this 
/// (for the older Sparkfun DEV-09628 USB Host Shield). See <a href="IMG_8837.JPG">this photo</a> showing Duemilanove, 
/// WiShield and USB Host Shield stack.
/// \code
/// // For USB Host Shield DEV-09628 interop with WiShield
/// #define MAX_SS      6
/// #define MAX_INT     3
/// #define MAX_GPX     7
/// #define MAX_RESET   5
/// \endcode
///
/// This library has been tested with Duemilanove and SparkFun USB Host Shield DEV-09628, with the 
/// Logitech 'Dual Action' USB Game Pad P/N 863247-0010 compiled with arduino-0021 
/// on OpenSuSE 11.1 and avr-libc-1.6.1-1.15,
/// cross-avr-binutils-2.19-9.1, cross-avr-gcc-4.1.3_20080612-26.5.
///
/// \author  Mike McCauley (mikem@airspayce.com)
///
/// This software is Copyright (C) 2011 Mike McCauley. Use is subject to license
/// conditions. The main licensing options available are GPL V2 or Commercial:
/// 
/// \par Open Source Licensing GPL V2
/// This is the appropriate option if you want to share the source code of your
/// application with everyone you distribute it to, and you also want to give them
/// the right to share who uses it. If you wish to use this software under Open
/// Source Licensing, you must contribute all your source code to the open source
/// community in accordance with the GPL Version 2 when your application is
/// distributed. See http://www.gnu.org/copyleft/gpl.html
/// 
/// \par Commercial Licensing
/// This is the appropriate option if you are creating proprietary applications
/// and you are not prepared to distribute and share the source code of your
/// application. Contact info@airspayce.com for details.
///
/// \par Revision History
///
/// \version 1.0 Initial release
/// \version 1.1 Compiles under Arduino 1.0 PROVIDED: you modify (or update) 
///              the USB_Host_Shield library so it builds under 1.0 too. At the time of writing there 
///              was no update to USB_Host_Shield available for 1.0.
/// \version 1.2 Updated author and distribution location details to airspayce.com

// Copyright (C) 2010 Mike McCauley
// $Id: USBJoystick.h,v 1.2 2012/01/19 00:09:54 mikem Exp mikem $

#ifndef USBJoystick_h
#define USBJoystick_h

#include <Usb.h>
#include <inttypes.h>
#include "Max3421e_constants.h"

/////////////////////////////////////////////////////////////////////
/// \class USBJoystick USBJoystick.h <USBJoystick.h>
/// \brief USB Interface to USB joystick/game pad, such as Logitech 'Dual Action' game pad
///
/// \par Overview
///
/// A class that interfaces with a USB Joystick or game pad. 
/// It can be subclassed to get control when new input values are read, or when they actually change.
/// Alternatively, you can set up callback functions to be called 
/// when new input values are read, or when they actually change.
///
/// Input data such as stick positions, button presses and hat-switch directions are read from the device
/// and delivered to the calling
/// application.
///
/// USBJoystick uses the USB_Host_Shield library to initialise and interrogate a USB joystick or game pad.
/// When the task() function is called, an attempt to read new values from the joystick is made every 
/// USBJOYSTICK_POLL_INTERVAL milliseconds.
/// If successful the NewValue
/// virtual functions are called. These in turn call the ValueDidChange virtual functions if the value 
/// changed since last poll. After the poll the new values can also be read with the data accessor functions.
///
/// The virtual functions will call their respective callbacks if callback functions have been set up.
///
/// Therefore developers have several choices for getting input data from a device:
/// - subclass USBJoystick and override the *NewValue() and/or *ValueDidChange() functions.
/// - set callback functions with set*NewValueCallback() and set*ValueDidChangeCallback() functions
/// - Asynchronously read current values with the *Value() functions.
///
/// \par Input indexes
///
/// The joystick/game pad is considered to have a number of joysticks, buttons and hat switches. In order
/// to identifiy which joysticks, buttons or hat switch is being referred to in callbacks etc, an index number is used.
/// Macros can be used to refer to specific items by name, see the USBJOYSTICK_STICK_*, USBJOYSTICK_BUTTON_* and 
/// USBJOYSTICK_HAT_* macros. The Logitech Dula Action game pad has 4 sticks 
/// (really 2 physical sticks with vertical and horizontal movement), 13 buttons and 1 hat switch.
///
/// \par Input Device Values
///
/// Stick positions are represented as 8 bit unsigned integers:
///   - 0 is fully left or up
///   - 255 is fully right or down
///
/// Button positions are represented by booleans:
///  - true - button is pressed (in the case of MODE button, mode is enabled and the red light is on) 
///  - false - otherwise
///
/// Hat switch positions are represented as 8 bit unsigned integers.
/// The current hat position is one of USBJOYSTICK_HAT_POS_*.
class USBJoystick
{

#define USBJOYSTICK_NUM_STICKS          4
#define USBJOYSTICK_NUM_BUTTONS        13
#define USBJOYSTICK_NUM_HATS            1

/// These are valid values for the stick index in callbacks etc
#define USBJOYSTICK_STICK_LEFT_HORIZ    0
#define USBJOYSTICK_STICK_LEFT_VERT     1
#define USBJOYSTICK_STICK_RIGHT_HORIZ   2
#define USBJOYSTICK_STICK_RIGHT_VERT    3

/// These are valid values for the button index in callbacks etc
#define USBJOYSTICK_BUTTON_MODE         0
#define USBJOYSTICK_BUTTON_1            1
#define USBJOYSTICK_BUTTON_2            2
#define USBJOYSTICK_BUTTON_3            3
#define USBJOYSTICK_BUTTON_4            4
#define USBJOYSTICK_BUTTON_5            5
#define USBJOYSTICK_BUTTON_6            6
#define USBJOYSTICK_BUTTON_7            7
#define USBJOYSTICK_BUTTON_8            8
#define USBJOYSTICK_BUTTON_9            9
#define USBJOYSTICK_BUTTON_10          10
#define USBJOYSTICK_BUTTON_LEFT_STICK  11
#define USBJOYSTICK_BUTTON_RIGHT_STICK 12

// These are valid values for the hat index in callbacks etc
#define USBJOYSTICK_HAT_1               0

// These are valid values for the hat value in callbacks etc
#define USBJOYSTICK_HAT_POS_N           0
#define USBJOYSTICK_HAT_POS_NE          1
#define USBJOYSTICK_HAT_POS_E           2
#define USBJOYSTICK_HAT_POS_SE          3
#define USBJOYSTICK_HAT_POS_S           4
#define USBJOYSTICK_HAT_POS_SW          5
#define USBJOYSTICK_HAT_POS_W           6
#define USBJOYSTICK_HAT_POS_NW          7
#define USBJOYSTICK_HAT_POS_IDLE        8

// Milliseconds
#define USBJOYSTICK_POLL_INTERVAL     50

public:
    /// Constructor. 
    /// Stick positions are initialised to 0x80, buttons to false and hat switches to idle
    USBJoystick();

    /// Library initialisation.
    /// Must be called once during setup()
    /// Enables the USB hardware and delays 200msec
    void init();

    /// USB Device regular poll function.
    /// Attempts to read current data from the USB device.
    /// If successful, calls the *NewValue() and/or *ValueDidChange() functions, and any calbacks if so configured.
    /// This must be called as often as possible in your main loop().
    /// Calls the intenal poll() functi0on every USBJOYSTICK_POLL_INTERVAL milliseconds
    void run();

    /// Read the current value of a joystick
    /// \param[in] stick The stick index. One of USBJOYSTICK_STICK_*
    /// \return the most recently read stick position value
    uint8_t stickValue(uint8_t stick);

    /// Read the current value of a button
    /// param[in] button The button index. One of USBJOYSTICK_BUTTON_*
    /// \return the most recently read button position value
    boolean buttonValue(uint8_t button);

    /// Read the current value of a joystick
    /// \param[in] hat The stick index. One of USBJOYSTICK_HAT_*
    /// \return the most recently read hat position value, one of USBJOYSTICK_HAT_POS_*
    uint8_t hatValue(uint8_t hat);

    /// Set a callback to be called when a stick value is read from 
    /// the usb device. Overrides any previously set callback. May be called at any time
    /// to set or change the callback.
    /// \param[in] cb The new callback function pointer
    void setStickNewValueCallback(void (*cb)(uint8_t stick, uint8_t value));

    /// Set a callback to be called when a button value is read from 
    /// the usb device. Overrides any previously set callback. May be called at any time
    /// to set or change the callback.
    /// \param[in] cb The new callback function pointer
    void setButtonNewValueCallback(void (*cb)(uint8_t button, uint8_t value));

    /// Set a callback to be called when a hat value is read from 
    /// the usb device. Overrides any previously set callback. May be called at any time
    /// to set or change the callback.
    /// \param[in] cb The new callback function pointer
    void setHatNewValueCallback(void (*cb)(uint8_t hat, uint8_t value));

    /// Set a callback to be called when a stick value is read from 
    /// the usb device and the value has changed since the last poll. 
    /// Overrides any previously set callback. May be called at any time
    /// to set or change the callback.
    /// \param[in] cb The new callback function pointer
    void setStickValueDidChangeCallback(void (*cb)(uint8_t stick, uint8_t value));

    /// Set a callback to be called when a button value is read from 
    /// the usb device and the value has changed since the last poll. 
    /// Overrides any previously set callback. May be called at any time
    /// to set or change the callback.
    /// \param[in] cb The new callback function pointer
    void setButtonValueDidChangeCallback(void (*cb)(uint8_t button, uint8_t value));

    /// Set a callback to be called when a hat value is read from 
    /// the usb device and the value has changed since the last poll. 
    /// Overrides any previously set callback. May be called at any time
    /// to set or change the callback.
    /// \param[in] cb The new callback function pointer
    void setHatValueDidChangeCallback(void (*cb)(uint8_t hat, uint8_t value));

protected:

    /// Initialises the USB hardware.
    /// Internal use: should not be called directly.
    /// \return true if initialisation was successful
    boolean device_init();

    /// Polls the USB device for a new reading.
    /// Internal use: should not be called directly.
    void poll();

    /// This virtual function is called when a new stick value is read from the USB device
    /// Default implementation calls the apropriate callback (if set) then calls 
    /// stickValueDidChange() if the value changed since the last poll.
    /// Subclasses may override this to change get control for each value read and change 
    /// the default behaviour
    /// \param[in] stick The stick index, one of USBJOYSTICK_STICK_*
    /// \param[in] value The new stick value
    virtual void stickNewValue(uint8_t stick, uint8_t value);

    /// This virtual function is called when a new button value is read from the USB device
    /// Default implementation calls the apropriate callback (if set) then calls 
    /// buttonValueDidChange() if the value changed since the last poll.
    /// Subclasses may override this to change get control for each value read and change 
    /// the default behaviour
    /// \param[in] button The button index, one of USBJOYSTICK_BUTTON_*
    /// \param[in] value The new button value
    virtual void buttonNewValue(uint8_t button, boolean value);

    /// This virtual function is called when a new hat switch value is read from the USB device
    /// Default implementation calls the apropriate callback (if set) then calls 
    /// hatValueDidChange() if the value changed since the last poll.
    /// Subclasses may override this to change get control for each value read and change 
    /// the default behaviour
    /// \param[in] hat The hat switch index, one of USBJOYSTICK_HAT_*
    /// \param[in] value The new hat position value
    virtual void hatNewValue(uint8_t hat, uint8_t value);

    /// This virtual function is called when a stick value changes
    /// Default implementation calls the apropriate callback (if set)
    /// \param[in] stick The stick index, one of USBJOYSTICK_STICK_*
    /// \param[in] value The new stick value
    virtual void stickValueDidChange(uint8_t stick, uint8_t value);

    /// This virtual function is called when a button value changes
    /// Default implementation calls the apropriate callback (if set)
    /// \param[in] button The button index, one of USBJOYSTICK_BUTTON_*
    /// \param[in] value The new button value
    virtual void buttonValueDidChange(uint8_t button, boolean value);

    /// This virtual function is called when a hat switch value changes
    /// Default implementation calls the apropriate callback (if set)
    /// \param[in] hat The hat index, one of USBJOYSTICK_HAT_*
    /// \param[in] value The new hat switch value
    virtual void hatValueDidChange(uint8_t hat, uint8_t value);

private:
    // Stores current (last read) value for each stick and button
    uint8_t   _stick_value[USBJOYSTICK_NUM_STICKS];
    boolean   _button_value[USBJOYSTICK_NUM_BUTTONS];
    uint8_t   _hat_value[USBJOYSTICK_NUM_HATS];

    // Callback pointers
    void (*_stick_new_value_cb)(uint8_t stick, uint8_t value);
    void (*_button_new_value_cb)(uint8_t button, boolean value);
    void (*_hat_new_value_cb)(uint8_t hat, uint8_t value);
    void (*_stick_value_did_change_cb)(uint8_t stick, uint8_t value);
    void (*_button_value_did_change_cb)(uint8_t button, boolean value);
    void (*_hat_value_did_change_cb)(uint8_t hat, uint8_t value);

    // USB interface data
    EP_RECORD ep_record[2];
    uint8_t buf[8];
};

#endif 

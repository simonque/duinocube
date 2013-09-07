// Ardrone.cpp
//
// USB Interface to USB joystick/game pad, such as Logitech 'Dual Action' game pad
/// \author  Mike McCauley (mikem@airspayce.com)
///
// Copyright (C) 2011 Mike McCauley
// $Id: USBJoystick.cpp,v 1.2 2012/01/19 00:09:54 mikem Exp mikem $

#include "USBJoystick.h"
#include <Usb.h>

USB Usb;

/* keyboard data taken from configuration descriptor */
#define KBD_ADDR        1
#define KBD_EP          1
#define KBD_IF          0
#define EP_MAXPKTSIZE   8
#define EP_POLL         0x0a

// Masks and offsets for contents of USB read for Logitech
// gamepad joystick.
#define  JOY_INDEX_LEFT_X 0
#define  JOY_INDEX_LEFT_Y 1
#define  JOY_INDEX_RIGHT_X 2
#define  JOY_INDEX_RIGHT_Y 3
#define  JOY_INDEX_BUTTONS_1 4
#define  JOY_INDEX_BUTTONS_2 5
#define  JOY_INDEX_BUTTONS_3 6

// Masks for buf index 4
#define JOY_MASK_BUTTON_1 0x10
#define JOY_MASK_BUTTON_2 0x20
#define JOY_MASK_BUTTON_3 0x40
#define JOY_MASK_BUTTON_4 0x80
#define JOY_MASK_HAT      0x0f

// Masks for buf index 5
#define JOY_MASK_BUTTON_5 0x1
#define JOY_MASK_BUTTON_6 0x2
#define JOY_MASK_BUTTON_7 0x4
#define JOY_MASK_BUTTON_8 0x8
#define JOY_MASK_BUTTON_9 0x10
#define JOY_MASK_BUTTON_10 0x20
#define JOY_MASK_BUTTON_LEFT_STICK 0x40
#define JOY_MASK_BUTTON_RIGHT_STICK 0x80

// Masks for buf index 6
#define JOY_MASK_BUTTON_MODE 0x8

/////////////////////////////////////////////////////////////////////
USBJoystick::USBJoystick()
{
    // Initialise stick, button and hat states
    uint8_t i;
    for (i = 0; i < USBJOYSTICK_NUM_STICKS; i++)
	_stick_value[i] = 0x80;
    for (i = 0; i < USBJOYSTICK_NUM_BUTTONS; i++)
	_button_value[i] = false;
    for (i = 0; i < USBJOYSTICK_NUM_HATS; i++)
	_hat_value[i] = USBJOYSTICK_HAT_POS_IDLE;

    // Clear callback pointers
    _stick_new_value_cb = 0;
    _button_new_value_cb = 0;
    _hat_new_value_cb = 0;
    _stick_value_did_change_cb = 0;
    _button_value_did_change_cb = 0;
    _hat_value_did_change_cb = 0;
}

void USBJoystick::init()
{
    Usb.powerOn();
}

boolean USBJoystick::device_init()
{
    /* Initialize data structures */
    ep_record[0] = *(Usb.getDevTableEntry(0, 0));  // copy endpoint 0 parameters
    ep_record[1].MaxPktSize = EP_MAXPKTSIZE;
    ep_record[1].Interval   = EP_POLL;
    ep_record[1].sndToggle  = bmSNDTOG0;
    ep_record[1].rcvToggle  = bmRCVTOG0;
    Usb.setDevTableEntry(1, ep_record); // plug kbd.endpoint parameters to devtable

    // Configure device
    if (Usb.setConf(KBD_ADDR, 0, 1))
	return false;

    // Set boot protocol
    if (Usb.setProto(KBD_ADDR, 0, 0, 0))
	return false;

    return true;
}

void USBJoystick::run()
{
    unsigned long current_t = millis();
    static unsigned long next_t = 0;
    if (current_t > next_t)
    {
	Usb.Task();
	
	if (Usb.getUsbTaskState() == USB_STATE_CONFIGURING) 
	{  
	    //wait for addressing state
	    if (device_init())
		Usb.setUsbTaskState(USB_STATE_RUNNING);
	}
	else if (Usb.getUsbTaskState() == USB_STATE_RUNNING) 
	    poll();
	next_t = current_t + USBJOYSTICK_POLL_INTERVAL;
    }
}

void USBJoystick::poll()
{
    if (Usb.inTransfer(KBD_ADDR, KBD_EP, 8, (char*)buf, USB_NAK_NOWAIT) == 0)
    {
        uint8_t i;
#if 0
	// Print raw input
	for (i = 0; i < 8; i++)
	{
	    Serial.print(buf[i], HEX);
	    Serial.print(" ");
	}
	Serial.println("");
#endif
	// Stick values
	for (i = 0; i < 4; i++)
	    stickNewValue(i, buf[i]);

	// Button values
	// We call the mode button 0
	buttonNewValue(USBJOYSTICK_BUTTON_MODE,        (buf[JOY_INDEX_BUTTONS_3] & JOY_MASK_BUTTON_MODE)       ? true : false);

	buttonNewValue(USBJOYSTICK_BUTTON_1,           (buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_1)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_2,           (buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_2)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_3,           (buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_3)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_4,           (buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_BUTTON_4)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_5,           (buf[JOY_INDEX_BUTTONS_2] & JOY_MASK_BUTTON_5)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_6,           (buf[JOY_INDEX_BUTTONS_2] & JOY_MASK_BUTTON_6)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_7,           (buf[JOY_INDEX_BUTTONS_2] & JOY_MASK_BUTTON_7)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_8,           (buf[JOY_INDEX_BUTTONS_2] & JOY_MASK_BUTTON_8)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_9,           (buf[JOY_INDEX_BUTTONS_2] & JOY_MASK_BUTTON_9)          ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_10,          (buf[JOY_INDEX_BUTTONS_2] & JOY_MASK_BUTTON_10)         ? true : false);
	// We call the left and right stick buttons 11 and 12 respectively
	buttonNewValue(USBJOYSTICK_BUTTON_LEFT_STICK,  (buf[JOY_INDEX_BUTTONS_2] & JOY_MASK_BUTTON_LEFT_STICK) ? true : false);
	buttonNewValue(USBJOYSTICK_BUTTON_RIGHT_STICK, (buf[JOY_INDEX_BUTTONS_2] & JOY_MASK_BUTTON_RIGHT_STICK) ? true : false);

	hatNewValue(USBJOYSTICK_HAT_1, buf[JOY_INDEX_BUTTONS_1] & JOY_MASK_HAT);

    }
}

uint8_t USBJoystick::stickValue(uint8_t stick)
{
    return _stick_value[stick];
}

boolean USBJoystick::buttonValue(uint8_t button)
{
    return _button_value[button];
}

uint8_t USBJoystick::hatValue(uint8_t hat)
{
    return _hat_value[hat];
}

void USBJoystick::stickNewValue(uint8_t stick, uint8_t value)
{
    // Call the callback
    if (_stick_new_value_cb)
	(*_stick_new_value_cb)(stick, value);

    uint8_t old_value = _stick_value[stick];
    _stick_value[stick] = value;
    if (old_value != value)
	stickValueDidChange(stick, value);
}

void USBJoystick::buttonNewValue(uint8_t button, boolean value)
{
    // Call the callback
    if (_button_new_value_cb)
	(*_button_new_value_cb)(button, value);

    uint8_t old_value = _button_value[button];
    _button_value[button] = value;
    if (old_value != value)
	buttonValueDidChange(button, value);
}

void USBJoystick::hatNewValue(uint8_t hat, uint8_t value)
{
    // Call the callback
    if (_hat_new_value_cb)
	(*_hat_new_value_cb)(hat, value);

    uint8_t old_value = _hat_value[hat];
    _hat_value[hat] = value;
    if (old_value != value)
	hatValueDidChange(hat, value);
}

void USBJoystick::stickValueDidChange(uint8_t stick, uint8_t value)
{
    // Call the callback
    if (_stick_value_did_change_cb)
	(*_stick_value_did_change_cb)(stick, value);
}

void USBJoystick::buttonValueDidChange(uint8_t button, boolean value)
{
    // Call the callback
    if (_button_value_did_change_cb)
	(*_button_value_did_change_cb)(button, value);
}

void USBJoystick::hatValueDidChange(uint8_t hat, uint8_t value)
{
    // Call the callback
    if (_hat_value_did_change_cb)
	(*_hat_value_did_change_cb)(hat, value);
}

void USBJoystick::setStickNewValueCallback(void (*cb)(uint8_t stick, uint8_t value))
{
    _stick_new_value_cb = cb;
}
void USBJoystick::setButtonNewValueCallback(void (*cb)(uint8_t button, uint8_t value))
{
    _button_new_value_cb = cb;
}
void USBJoystick::setHatNewValueCallback(void (*cb)(uint8_t hat, uint8_t value))
{
    _hat_new_value_cb = cb;
}
void USBJoystick::setStickValueDidChangeCallback(void (*cb)(uint8_t stick, uint8_t value))
{
    _stick_value_did_change_cb = cb;
}
void USBJoystick::setButtonValueDidChangeCallback(void (*cb)(uint8_t button, uint8_t value))
{
    _button_value_did_change_cb = cb;
}
void USBJoystick::setHatValueDidChangeCallback(void (*cb)(uint8_t hat, uint8_t value))
{
    _hat_value_did_change_cb = cb;
}

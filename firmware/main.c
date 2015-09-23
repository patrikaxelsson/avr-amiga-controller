// ======================================================================
// avr-amiga-controller - emulates amiga kb&mouse in hardware
//
// Copyright (C) 2013 Alistair Buxton <a.j.buxton@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ======================================================================

#include <util/delay.h>

#include "main.h"
#include "../keytable.h"

#if BOARD == BOARD_MINIMUS
    #define KEYB_PORT PORTD
    #define KEYB_DDR DDRD
    #define KEYB_CLK 0
    #define KEYB_DATA 1
    #define KEYB_INT INT1
    
    #define MOUSEMV_PORT PORTB
    #define MOUSEMV_DDR DDRB
    #define MOUSEMV_MASK 0xf0
    #define MOUSEMV_OFFSET 4
    
    #define MOUSEBTN_PORT PORTC
    #define MOUSEBTN_DDR DDRC
    #define MOUSEBTN_MASK 0x70
    #define MOUSEBTN_OFFSET 4

// Pro Micro, suppport files in Board directory
#elif BOARD == BOARD_USER
    #define KEYB_PORT PORTD
    #define KEYB_DDR DDRD
    #define KEYB_CLK 0
    #define KEYB_DATA 1
    #define KEYB_INT INT1
    
    #define MOUSEMV_PORT PORTF
    #define MOUSEMV_DDR DDRF
    #define MOUSEMV_MASK 0xf0
    #define MOUSEMV_OFFSET 4
    
    #define MOUSEBTN_PORT PORTB
    #define MOUSEBTN_DDR DDRB
    #define MOUSEBTN_MASK 0x70
    #define MOUSEBTN_OFFSET 4
#endif

volatile uint8_t got_sync;

inline static void kclock(void) {
    _delay_us(20);
    KEYB_PORT &= ~_BV(KEYB_CLK);
    _delay_us(20);
    KEYB_PORT |= _BV(KEYB_CLK);
    _delay_us(20);
}

inline static void kdat(uint8_t b) {
    if(b)
        KEYB_PORT &= ~_BV(KEYB_DATA);
    else
        KEYB_PORT |= _BV(KEYB_DATA);
    kclock();
}


ISR(INT1_vect)
{
    got_sync = 1;
    EIMSK &= ~_BV(KEYB_INT); // disable INT1
} 

uint8_t sync(void) {
    EIMSK &= ~_BV(KEYB_INT); // disable INT1
    KEYB_PORT &= ~_BV(KEYB_DATA);
    KEYB_DDR &= ~_BV(KEYB_DATA);
    _delay_us(20);
    EIFR = _BV(KEYB_INT); // clear INT1 interrupt
    got_sync = 0;
    EIMSK |= _BV(KEYB_INT); // enable INT1
    _delay_us(170);
    if(got_sync) return true;

    while(1)
    {
        EIMSK &= ~_BV(KEYB_INT); // disable INT1
        KEYB_PORT &= ~_BV(KEYB_DATA);
        KEYB_DDR |= _BV(KEYB_DATA);
        kclock();
        KEYB_DDR &= ~_BV(KEYB_DATA);
        KEYB_PORT |= _BV(KEYB_DATA);
        _delay_us(20);
        EIFR = _BV(KEYB_INT);
        got_sync = 0;
        EIMSK |= _BV(KEYB_INT); // enable INT1
        _delay_us(120);
        if(got_sync) return false;
    }
}


volatile uint8_t xc;
volatile uint8_t yc;

volatile int16_t dx;
volatile int16_t dy;

void mouse(void)
{
    uint8_t tmp;
    if(dx == 0 && dy == 0) {
        // disable timer interrupt
        return;
    }
    if(dx < 0) { dx++; xc--; }
    else if(dx > 0) { dx--; xc++; }
    if(dy < 0) { dy++; yc--; }
    else if(dy > 0) { dy--; yc++; }
    tmp = ((xc>>1)&3) | (((yc>>1)&3)<<2);
    tmp ^= (tmp>>1)&5;
    MOUSEMV_DDR = (MOUSEMV_DDR & ~MOUSEMV_MASK) | (tmp << MOUSEMV_OFFSET);
    _delay_us(100);
}

void keyboard(uint8_t k)
{
    if(k == 0xff) return;
    KEYB_PORT |= _BV(KEYB_DATA);
    KEYB_DDR |= _BV(KEYB_DATA);
    kdat((k>>6)&1);
    kdat((k>>5)&1);
    kdat((k>>4)&1);
    kdat((k>>3)&1);
    kdat((k>>2)&1);
    kdat((k>>1)&1);
    kdat((k>>0)&1);
    kdat((k>>7)&1);
    if(sync()) LEDs_ToggleLEDs(LEDS_LED2);
    else LEDs_ToggleLEDs(LEDS_LED1);
}



void SetupHardware(void)
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    MOUSEBTN_DDR &= ~MOUSEBTN_MASK;
    MOUSEBTN_PORT &= ~MOUSEBTN_MASK;
    MOUSEMV_DDR &= ~MOUSEMV_MASK;
    MOUSEMV_PORT &= ~MOUSEMV_MASK;

    KEYB_DDR  |= _BV(KEYB_CLK);
    KEYB_PORT |= _BV(KEYB_CLK);

    EIMSK &= ~_BV(KEYB_INT); // disable INT1
    EIFR = _BV(KEYB_INT); // clear INT1 interrupt

    sei();
    sync();
    keyboard(0xfd);
    keyboard(0xfe);
    keyboard(0x35);
    cli();

    LEDs_Init();
    USB_Init();

    /* Hardware Initialization */
}

int main(void)
{
    SetupHardware();
    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);

    sei();

    for (;;) {
        USB_USBTask();
    }
}

void EVENT_USB_Device_Connect(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

void EVENT_USB_Device_Disconnect(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;
    LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}



void EVENT_USB_Device_ControlRequest(void)
{
//    unsigned char buf[0x4] = {0, 1, 2, 3};

    static uint8_t hmm = 0;

    switch (USB_ControlRequest.bmRequestType) {
    case 0x40:
        
        switch (USB_ControlRequest.bRequest) {
        case REQ_MOUSE_REL:
            dx += (int16_t)USB_ControlRequest.wValue;
            dy += (int16_t)USB_ControlRequest.wIndex;
            while(dx != 0 || dy != 0) mouse();
            Endpoint_ClearSETUP();
            Endpoint_ClearIN();
            break;
        case REQ_MOUSE_BTN:
            MOUSEBTN_DDR = (MOUSEBTN_DDR & ~MOUSEBTN_MASK) | (USB_ControlRequest.wValue << MOUSEBTN_OFFSET);
            Endpoint_ClearSETUP();
            Endpoint_ClearIN();
            break;
        case REQ_KEYBOARD:
            if(USB_ControlRequest.wValue == 115 && USB_ControlRequest.wIndex) {
                KEYB_PORT &= ~_BV(KEYB_CLK);
                _delay_ms(500);
                KEYB_PORT |= _BV(KEYB_CLK);
            }
            if(USB_ControlRequest.wValue < 0xa0) {
                keyboard(keycodes[USB_ControlRequest.wValue] | (USB_ControlRequest.wIndex ? 0x00 : 0x80));
            }
            Endpoint_ClearSETUP();
            Endpoint_ClearIN();
            break;
        }
        break;
    }
}

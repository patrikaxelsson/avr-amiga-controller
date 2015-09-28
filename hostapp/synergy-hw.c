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

#include "uSynergy.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

#include <usb.h>

#include "../common.h"
#include "../keytable.h"

int sockfd, portno;
struct sockaddr_in serv_addr;
struct hostent *server;

usb_dev_handle *usbhandle;
static usb_dev_handle *find_device(unsigned short, unsigned short);
static usb_dev_handle *find_device_retry(unsigned short, unsigned short);

char name[] = "amiga";

int init()
{
    usb_init();
    usbhandle = find_device_retry(VENDOR_ID, PRODUCT_ID);
    if(usbhandle != NULL)
        return 0;
    return -1;
}

static usb_dev_handle *find_device(unsigned short vid, unsigned short pid)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_dev_handle *handle = 0;

    usb_find_busses();
    usb_find_devices();
    for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == vid
                && dev->descriptor.idProduct == pid) {

                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if (!handle) {
                    fprintf(stderr,
                            "Warning: cannot open USB device: %s\n",
                            usb_strerror());
                    continue;
                }
                fprintf(stderr, "Connected\n");
            }
        }
        if (handle)
            break;
    }
    return handle;
}

usb_dev_handle *find_device_retry(unsigned short vid, unsigned short pid)
{
    if (usbhandle != NULL) {
        usb_close(usbhandle);
        usbhandle = NULL;
    }
    while ((usbhandle = find_device(vid, pid)) == NULL) {
        fprintf(stderr,
                "Could not find USB device with vid=0x%x pid=0x%x, retrying in 2 seconds\n",
                vid, pid);
        usleep(2 * 1000 * 1000);
    }
    return usbhandle;
}

int usb_request(uint8_t bRequest, uint16_t wValue, uint16_t wIndex)
{
    int result = usb_control_msg(usbhandle, 0x40, bRequest, wValue, wIndex, 0, 0, 500);
    if (result < 0) {
        fprintf(stderr, "Problem communicating with USB device, reconnecting\n");
        usbhandle = find_device_retry(VENDOR_ID, PRODUCT_ID);
    }
    return result;
}

int usb_send_amiga_key(uint8_t amigaKey, uint8_t keyUp) {
    const uint8_t amigaKeyUpMask = 0x80;
    int result;
    
    result = usb_request(REQ_KEYBOARD, amigaKey | (keyUp ? amigaKeyUpMask : 0x00) , 0);
    printf("send_amiga_key, amigaKey=%2x keyUp=%d - %d\n", amigaKey, keyUp, result);
}

uSynergyBool s_connect(uSynergyCookie cookie)
{

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("No sock");
        return USYNERGY_FALSE;
    }

    server = gethostbyname("localhost");
    if (server == NULL) {
        perror("DNS fail");
        return USYNERGY_FALSE;
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(24800);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connect failed");
        USYNERGY_FALSE;
    }

    printf("connect\n");

    return USYNERGY_TRUE;
}

uSynergyBool s_send(uSynergyCookie cookie, const uint8_t * buffer, int length)
{
    int n;

    n = write(sockfd, buffer, length);
    if (n < 0) {
        perror("Write failed:");
        return USYNERGY_FALSE;
    }
    return USYNERGY_TRUE;
}

uSynergyBool s_receive(uSynergyCookie cookie, uint8_t * buffer, int maxLength,
                       int *outLength)
{
    int n;

    *outLength = read(sockfd, buffer, maxLength);
    if (*outLength < 0) {
        perror("Read failed:");
        return USYNERGY_FALSE;
    }
    return USYNERGY_TRUE;
}

void s_sleep(uSynergyCookie cookie, int timeMs)
{
    usleep(timeMs * 1000);
}

uint32_t s_getTime()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000) + (now.tv_usec / 1000);
}

void s_trace(uSynergyCookie cookie, const char *text)
{
    printf("%s\n", text);
}

void s_screenActive(uSynergyCookie cookie, uSynergyBool active)
{
    printf("screenActive, active=%d\n", active);
    
    // Hack for releasing any modifier keys used when switching from Amiga to PC
    if (active == 0) {
        uint8_t amigaModifierKey;
        for (amigaModifierKey = 0x60; amigaModifierKey < 0x68; amigaModifierKey++) {
            usb_send_amiga_key(amigaModifierKey, 1);
        }
    }
}

void s_mouse(uSynergyCookie cookie, uint16_t x, uint16_t y, int16_t wheelX,
             int16_t wheelY, uSynergyBool buttonLeft, uSynergyBool buttonRight,
             uSynergyBool buttonMiddle)
{
    static uint8_t old_mstate = 0;
    uint8_t mstate =
        (buttonLeft ? 1 : 0) | (buttonRight ? 2 : 0) | (buttonMiddle ? 4 : 0);
    int result;

    if (mstate != old_mstate) {
        result = usb_request(REQ_MOUSE_BTN, mstate, 0);
        printf("mouse, button=%d,%d,%d - %d\n", buttonLeft,
               buttonMiddle, buttonRight, result);
        old_mstate = mstate;
    }
}

void s_mouserel(uSynergyCookie cookie, int16_t x, int16_t y)
{
    int result;
    result = usb_request(REQ_MOUSE_REL, x, y);
    printf("mouse, rel=%d, %d - %d\n", x, y, result);
}

void s_keyboard(uSynergyCookie cookie, uint16_t key, uint16_t modifiers, uSynergyBool down,
                uSynergyBool repeat)
{
    const uint8_t amigaLeftShift = 0x60;
    const uint8_t amigaCapsLock = 0x62;
    const uint8_t amigaCursorDown = 0x4d;
    const uint8_t amigaCursorUp = 0x4c;
    const uint8_t amigaCursorLeft = 0x4f;
    const uint8_t amigaCursorRight = 0x4e;

    if (!repeat) {
        switch (key) {
            case 0x75: // Page down
                usb_send_amiga_key(amigaLeftShift, !down);
                usb_send_amiga_key(amigaCursorDown, !down);
                break;
            case 0x70: // Page up
                usb_send_amiga_key(amigaLeftShift, !down);
                usb_send_amiga_key(amigaCursorUp, !down);
                break;
            case 0x6e: // Home
                usb_send_amiga_key(amigaLeftShift, !down);
                usb_send_amiga_key(amigaCursorLeft, !down);
                break;
            case 0x73: // End
                usb_send_amiga_key(amigaLeftShift, !down);
                usb_send_amiga_key(amigaCursorRight, !down);
                break;
            case 0x42: // Caps lock
                if (down) {
                    // If caps lock is pressed down and modifiers has not caps lock set, then
                    // send caps down to amiga, else send caps up
                    uint8_t capsKeyUp = modifiers & USYNERGY_MODIFIER_CAPSLOCK ? 1 : 0;
                    usb_send_amiga_key(amigaLeftShift, capsKeyUp);
                }
                break;
            default:
                if (key < sizeof(keycodes)) {
                    uint8_t amigaKey = keycodes[key];
                    if (amigaKey != 0xff)
                        usb_send_amiga_key(amigaKey, !down);
                }
        }
        printf("keyboard, key=%2x down=%d modifiers=%4x\n", key, down, modifiers);
    }
}

void s_joystick(uSynergyCookie cookie, uint8_t joyNum, uint16_t buttons,
                int8_t leftStickX, int8_t leftStickY, int8_t rightStickX,
                int8_t rightStickY)
{
    printf("joystick, left=%d,%d right=%d,%d\n", leftStickX, leftStickY,
           rightStickX, rightStickY);
}

void s_clipboard(uSynergyCookie cookie, enum uSynergyClipboardFormat format,
                 const uint8_t * data, uint32_t size)
{
    printf("clipboard, size=%d\n", size);
}

int main(char *argv, int argc)
{
    uSynergyContext context;
    uSynergyInit(&context);

    context.m_connectFunc = &s_connect;
    context.m_sendFunc = &s_send;
    context.m_receiveFunc = &s_receive;
    context.m_sleepFunc = &s_sleep;
    context.m_getTimeFunc = &s_getTime;
    context.m_traceFunc = &s_trace;
    context.m_screenActiveCallback = &s_screenActive;
    context.m_mouseCallback = &s_mouse;
    context.m_mouseRelativeCallback = &s_mouserel;
    context.m_keyboardCallback = &s_keyboard;
    context.m_joystickCallback = &s_joystick;
    context.m_clipboardCallback = &s_clipboard;

    context.m_clientName = name;
    context.m_clientWidth = 1000;
    context.m_clientHeight = 1000;

    init();

    for (;;) {
        uSynergyUpdate(&context);
    }
}

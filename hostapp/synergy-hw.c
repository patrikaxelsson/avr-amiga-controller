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
#include <getopt.h>

#include <usb.h>

#include "../common.h"
#include "keytables.h"

enum ServerType {
    Linux,
    Mac
};

enum ServerType serverType = Linux;

uint8_t *keycodes = linux_keycodes;

int sockfd = 0;
struct sockaddr_in serv_addr;
struct hostent *server;

usb_dev_handle *usbhandle;
static usb_dev_handle *find_device(unsigned short, unsigned short, char *);

char *usbSerialNum = "";
int debugLevel = 0;

int init()
{
    usb_init();
    usbhandle = find_device(VENDOR_ID, PRODUCT_ID, usbSerialNum);
    return 0;
}

static usb_dev_handle *find_device(unsigned short vid, unsigned short pid, char *serial)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    usb_dev_handle *handle = 0;
    char tmpSerialNum[64];

    usb_find_busses();
    usb_find_devices();
    for (bus = usb_busses; bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == vid
                && dev->descriptor.idProduct == pid) {

                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if (!handle) {
                    fprintf(stderr, "Warning: cannot open USB device: %s\n", usb_strerror());
                    continue;
                }
                
                tmpSerialNum[0] = '\0';
                if (dev->descriptor.iSerialNumber > 0) {
                    usb_get_string_simple(handle, dev->descriptor.iSerialNumber, tmpSerialNum, sizeof(tmpSerialNum));
                }

                if (strlen(serial) > 0 &&
                    strcmp(serial, tmpSerialNum) != 0) {
                    fprintf(stderr, "Found USB device with correct vid and pid but serial '%s' doesn't match, skipping\n", tmpSerialNum);
                    usb_close(handle);
                    handle = 0;
                }
                else
                    fprintf(stderr, "Connected to USB device with serial '%s'\n", tmpSerialNum);
            }
        }
        if (handle)
            break;
    }
    if (!handle)
        fprintf(stderr, "Warning: Could not find USB device with vid=0x%x pid=0x%x serial='%s'\n", vid, pid, serial);
    return handle;
}

int usb_request(uint8_t bRequest, uint16_t wValue, uint16_t wIndex)
{
    if (usbhandle == NULL) {
        usbhandle = find_device(VENDOR_ID, PRODUCT_ID, usbSerialNum);
        if (usbhandle == NULL) {
            return -1;
        }

    }
    int result = usb_control_msg(usbhandle, 0x40, bRequest, wValue, wIndex, 0, 0, 500);
    if (result < 0) {
        fprintf(stderr, "Warning: Problem communicating with USB device, closing handle\n");
        usb_close(usbhandle);
        usbhandle = NULL;
    }
    return result;
}

int usb_send_amiga_key(uint8_t amigaKey, uint8_t keyUp) {
    static int count = 0;
    const uint8_t amigaKeyUpMask = 0x80;
    int result;
    
    result = usb_request(REQ_KEYBOARD, amigaKey | (keyUp ? amigaKeyUpMask : 0x00) , 0);
    if (debugLevel) printf("send_amiga_key, amigaKey=%2x keyUp=%d count=%d - %d\n", amigaKey, keyUp, count++, result);

    return result;
}

uSynergyBool s_connect(uSynergyCookie cookie)
{
    // To be able to handle reconnects cleanly
    if(sockfd) {
        close(sockfd);
    }

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
        perror("Connecting to synergy server");
        // A primitive limit to not make it reconnect as fast as possible
        usleep(1000000);
        return USYNERGY_FALSE;
    }
    
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
    if (debugLevel) printf("screenActive, active=%d\n", active);
    
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
        if (debugLevel) printf("mouse, button=%d,%d,%d - %d\n", buttonLeft,
               buttonMiddle, buttonRight, result);
        old_mstate = mstate;
    }
}

void s_mouserel(uSynergyCookie cookie, int16_t x, int16_t y)
{
    int result;
    result = usb_request(REQ_MOUSE_REL, x, y);
    if (debugLevel) printf("mouse, rel=%d, %d - %d\n", x, y, result);
}

int doSpecialKeyActions(uint16_t key, uint16_t modifiers, uSynergyBool down) {
    const uint8_t amigaLeftShift = 0x60;
    const uint8_t amigaCapsLock = 0x62;
    const uint8_t amigaCursorDown = 0x4d;
    const uint8_t amigaCursorUp = 0x4c;
    const uint8_t amigaCursorLeft = 0x4f;
    const uint8_t amigaCursorRight = 0x4e;

    int didSpecialStuff = 0;

    switch(serverType) {
        case Linux:
            switch (key) {
                case 0x75: // Page down
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorDown, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x70: // Page up
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorUp, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x6e: // Home
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorLeft, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x73: // End
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorRight, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x42: // Caps lock
                    if (down) {
                        // If caps lock is pressed down and modifiers has not caps lock set, then
                        // send caps down to amiga, else send caps up
                        uint8_t capsKeyUp = modifiers & USYNERGY_MODIFIER_CAPSLOCK ? 1 : 0;
                        usb_send_amiga_key(amigaCapsLock, capsKeyUp);
                    }
                    didSpecialStuff = 1;
                    break;
            }
            break;
        case Mac:
            switch (key) {
                case 0x7a: // Page down
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorDown, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x75: // Page up
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorUp, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x74: // Home
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorLeft, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x78: // End
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorRight, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x3a: // Caps lock
                    if (down) {
                        // If caps lock is pressed down and modifiers has not caps lock set, then
                        // send caps down to amiga, else send caps up
                        uint8_t capsKeyUp = modifiers & USYNERGY_MODIFIER_CAPSLOCK ? 0 : 1;
                        usb_send_amiga_key(amigaCapsLock, capsKeyUp);
                    }
                    didSpecialStuff = 1;
                    break;
            }
            break;
    }
    return didSpecialStuff; 
}

void s_keyboard(uSynergyCookie cookie, uint16_t key, uint16_t modifiers, uSynergyBool down,
                uSynergyBool repeat)
{
    if(debugLevel > 1)
        printf("keyboard - incoming, key=%4x modifiers=%04x down=%d\n", key, modifiers, down);

    if (!repeat) {
        if(!doSpecialKeyActions(key, modifiers, down)) {
            if (key < 0xA0) {
                uint8_t amigaKey = keycodes[key];
                if (amigaKey != 0xff) {
                    usb_send_amiga_key(amigaKey, !down);
                }
                else {
                    if (debugLevel) printf("keyboard - ignored, key=%4x modifiers=%04x down=%d\n", key, modifiers, down);
                }
            }
            else {
                if (debugLevel) printf("keyboard - unmapped, key=%4x modifiers=%04x down=%d\n", key, modifiers, down);
            }
        }
    }
}


void s_joystick(uSynergyCookie cookie, uint8_t joyNum, uint16_t buttons,
                int8_t leftStickX, int8_t leftStickY, int8_t rightStickX,
                int8_t rightStickY)
{
    if (debugLevel) printf("joystick, left=%d,%d right=%d,%d\n", leftStickX, leftStickY,
           rightStickX, rightStickY);
}

void s_clipboard(uSynergyCookie cookie, enum uSynergyClipboardFormat format,
                 const uint8_t * data, uint32_t size)
{
    if (debugLevel) printf("clipboard, size=%d\n", size);
}

int main(int argc, char **argv)
{
    uSynergyContext context;
    int option = 0;
    int keyCodesFromStdin = 0;
    char *synergyClientName = "amiga";

    while ((option = getopt(argc, argv,"n:s:t:d:h")) != -1) {
        switch (option) {
            case 'n':
                synergyClientName = optarg; 
                break;
            case 't':
                if(!strcmp(optarg, "linux")) {
                    keycodes = linux_keycodes;
                    serverType = Linux;
                }
                else if(!strcmp(optarg, "mac")) {
                    keycodes = mac_keycodes;
                    serverType = Mac;
                }
                break;
            case 's':
                usbSerialNum = optarg;
                break;
            case 'd':
                debugLevel = atoi(optarg);
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-n synergyClientName] [-t synergyServerType linux/mac][-s usbSerialNum] [-d debugLevel]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (optind < argc && strcmp(argv[optind], "-") == 0) {
        keyCodesFromStdin = 1;
    }

    printf("Server type: %s\n", serverType == Linux ? "linux" : "mac");

    init();

    if (keyCodesFromStdin) {
        int c;
        fprintf(stderr, "Will send stdin as raw amiga keycodes to the avr\n");
        while((c = getchar())) {
            if (c < 0 || c == EOF) exit(0);
            usb_send_amiga_key(c, c & 0x80);
        }
    }

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
    context.m_clientName = synergyClientName;
    context.m_clientWidth = 1000;
    context.m_clientHeight = 1000;

    for (;;) {
        uSynergyUpdate(&context);
    }
}

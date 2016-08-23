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

#include <libusb-1.0/libusb.h>

#include "../common.h"
#include "keytables.h"

enum ServerType {
    Linux,
    Mac,
    Windows,
    UnknownType
};

enum ServerType serverType = Linux;
uint8_t *keycodes = NULL;

int sockfd = 0;
struct sockaddr_in serv_addr;
struct hostent *server;

libusb_device_handle *usbhandle = NULL;
static libusb_device_handle *find_device(const unsigned short, const unsigned short, const char *);

char *usbSerialNum = "";
int debugLevel = 0;

unsigned int amigaMouseResWidth = 724 * 2;
unsigned int amigaMouseResHeight = 283 * 4;

int matchesSerial(const char *serial, const char *usbSerial)
{
    return 0 == strlen(serial) || 0 == strcmp(serial, usbSerial);
}

static libusb_device_handle *find_device(const unsigned short vid, const unsigned short pid, const char *serial)
{
    libusb_device_handle *handle = NULL;
    libusb_device **devices;
    ssize_t numDevices = libusb_get_device_list(NULL, &devices);
    ssize_t i = 0;

    for (i = 0; i < numDevices; i++) {
        struct libusb_device_descriptor descriptor;
        if (0 == libusb_get_device_descriptor(devices[i], &descriptor)) {
            if (vid == descriptor.idVendor && pid == descriptor.idProduct) {
                if (0 == libusb_open(devices[i], &handle)) {
                    char usbSerial[64];
                    if (0 == descriptor.iSerialNumber || 
                        0 == libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber, (unsigned char *)usbSerial, sizeof(usbSerial)))
                        usbSerial[0] = '\0';
   
                    if (matchesSerial(serial, usbSerial)) {
                        fprintf(stderr, "Connected to USB device with serial='%s'\n", usbSerial);
                        break;
                    }
                    else {
                        libusb_close(handle);
                        handle = NULL;
                    }
                }
            }
        }
    }
    if (NULL == handle)
        fprintf(stderr, "Warning: Could not find USB device with vid=0x%x pid=0x%x serial='%s'\n", vid, pid, serial);

    libusb_free_device_list(devices, 1);
    return handle;
}

void listUsbDeviceMatches(const unsigned short vid, const unsigned short pid)
{
    libusb_device **devices;
    ssize_t numDevices = libusb_get_device_list(NULL, &devices);
    ssize_t i = 0;

    for (i = 0; i < numDevices; i++) {
        struct libusb_device_descriptor descriptor;
        if (0 == libusb_get_device_descriptor(devices[i], &descriptor)) {
            if (vid == descriptor.idVendor && pid == descriptor.idProduct) {
                libusb_device_handle *handle;
                if (0 == libusb_open(devices[i], &handle)) {
                    char usbSerial[64];
                    if (0 == descriptor.iSerialNumber || 
                        0 == libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber, (unsigned char *)usbSerial, sizeof(usbSerial)))
                        usbSerial[0] = '\0';
  
                    fprintf(stderr, "USB device pid/vid match: vid=0x%04x pid=0x%04x serial='%s'\n", descriptor.idVendor, descriptor.idProduct, usbSerial);
                    libusb_close(handle);
                }
            }
        }
    }
    libusb_free_device_list(devices, 1);
}

int usb_request(uint8_t bRequest, uint16_t wValue, uint16_t wIndex)
{
    if (usbhandle == NULL) {
        usbhandle = find_device(VENDOR_ID, PRODUCT_ID, usbSerialNum);
        if (usbhandle == NULL) {
            return -1;
        }

    }
    int result = libusb_control_transfer(usbhandle, 0x40, bRequest, wValue, wIndex, 0, 0, 1000);
    if (result < 0) {
        fprintf(stderr, "Warning: Problem communicating with USB device, closing handle: %s\n", libusb_strerror(result));
        libusb_close(usbhandle);
        usbhandle = NULL;
    }
    return result;
}

void usb_send_amiga_key(uint8_t amigaKey, uint8_t keyUp) {
    static int count = 0;
    const uint8_t amigaKeyUpMask = 0x80;
    
    if(0 == usb_request(REQ_KEYBOARD, amigaKey | (keyUp ? amigaKeyUpMask : 0x00) , 0)) {
        if (debugLevel) fprintf(stderr, "send_amiga_key, amigaKey=0x%02x keyUp=%d count=%d\n", amigaKey, keyUp, count++);
    }
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
    fprintf(stderr, "synergy, %s\n", text);
}

int lastMouseMoveWasRelative = 0;
uint16_t mouse_x = 0;
uint16_t mouse_y = 0;

int mouseAbsHasMoved(uint16_t x, uint16_t y) {
    const int16_t diff_x = x - mouse_x;
    const int16_t diff_y = y - mouse_y;
    return diff_x != 0 || diff_y != 0;
}

void mouseAbsMove(uint16_t x, uint16_t y) {
    const int16_t diff_x = x - mouse_x;
    const int16_t diff_y = y - mouse_y;

    if(0 == usb_request(REQ_MOUSE_REL, diff_x, diff_y)) {
        mouse_x = x;
        mouse_y = y;
        if (debugLevel > 1) fprintf(stderr, "mouse, abs.move to x=%d y=%d\n", x, y);
    }
}

void mouseAbsResetToNearestCorner(uint16_t x, uint16_t y) {
    int mouseMoveToReset_x = x < (amigaMouseResWidth / 2) ? -amigaMouseResWidth : amigaMouseResWidth;
    int mouseMoveToReset_y = y < (amigaMouseResHeight / 2) ? -amigaMouseResHeight : amigaMouseResHeight;
        
    if(0 == usb_request(REQ_MOUSE_REL, mouseMoveToReset_x, mouseMoveToReset_y)) {
        mouse_x = mouseMoveToReset_x < 0 ? 0 : amigaMouseResWidth - 1;
        mouse_y = mouseMoveToReset_y < 0 ? 0 : amigaMouseResHeight - 1;
        if (debugLevel) fprintf(stderr, "mouse, abs.reset to x=%d y=%d\n", mouse_x, mouse_y);
    }
}

void mouseAbsMoveToNearestSide(uint16_t x, uint16_t y) {
    const uint16_t distanceLeft = x;
    const uint16_t distanceRight = (amigaMouseResWidth - 1) - x;
    const uint16_t distanceUp = y;
    const uint16_t distanceDown = (amigaMouseResHeight - 1) - y;

    if(distanceLeft < distanceRight && distanceLeft < distanceDown && distanceLeft < distanceUp)
        mouseAbsMove(0, y);
    else if(distanceRight < distanceLeft && distanceRight < distanceDown && distanceRight < distanceUp)
        mouseAbsMove(amigaMouseResWidth - 1, y);
    else if(distanceUp < distanceLeft && distanceUp < distanceRight && distanceUp < distanceDown)
        mouseAbsMove(x, 0);
    else if(distanceDown < distanceLeft && distanceDown < distanceRight && distanceDown < distanceUp)
        mouseAbsMove(x, amigaMouseResHeight - 1);
}

void s_screenActive(uSynergyCookie cookie, uSynergyBool active, int16_t x, int16_t y, uint16_t modifiers)
{
    if (debugLevel) fprintf(stderr, "screenActive, active=%d x=%d y=%d modifiers=0x%04x lastMouseMoveWasRelative=%d\n", active, x, y, modifiers, lastMouseMoveWasRelative);
    
    // Hack for releasing any modifier keys used when switching from Amiga to PC
    if (active == 0) {
        uint8_t amigaModifierKey;
        for (amigaModifierKey = 0x60; amigaModifierKey < 0x68; amigaModifierKey++) {
            usb_send_amiga_key(amigaModifierKey, 1);
        }
        if(!lastMouseMoveWasRelative) {
            // Using globals here as we get no coordinates when leaving the screen
            mouseAbsMoveToNearestSide(mouse_x, mouse_y);
        }
    } 
    else {
        if(!lastMouseMoveWasRelative) {
            mouseAbsResetToNearestCorner(x, y); 
            mouseAbsMove(x, y);
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

    if(mouseAbsHasMoved(x, y)) {
        if(lastMouseMoveWasRelative) {
            mouseAbsResetToNearestCorner(x, y);
            lastMouseMoveWasRelative = 0;
        }
        mouseAbsMove(x, y);
    }

    if (mstate != old_mstate) {
        if(0 == usb_request(REQ_MOUSE_BTN, mstate, 0)) {
            old_mstate = mstate;
            lastMouseMoveWasRelative = 0;
            if (debugLevel > 1) fprintf(stderr, "mouse, button=%d,%d,%d\n", buttonLeft, buttonMiddle, buttonRight);
        }
    }
}

void s_mouserel(uSynergyCookie cookie, int16_t x, int16_t y)
{
    if(0 == usb_request(REQ_MOUSE_REL, x, y)) {
        lastMouseMoveWasRelative = 1;
        if (debugLevel) fprintf(stderr, "mouse, rel.move x=%d y=%d\n", x, y);
    }
}

int doSpecialKeyActions(uint16_t key, uint16_t modifiers, uSynergyBool down) {
    static const uint8_t amigaLeftShift = 0x60;
    static const uint8_t amigaCapsLock = 0x62;
    static const uint8_t amigaCursorDown = 0x4d;
    static const uint8_t amigaCursorUp = 0x4c;
    static const uint8_t amigaCursorLeft = 0x4f;
    static const uint8_t amigaCursorRight = 0x4e;

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
        case Windows:
            switch (key) {
                case 0x151: // Page down
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorDown, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x149: // Page up
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorUp, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x147: // Home
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorLeft, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x14f: // End
                    usb_send_amiga_key(amigaLeftShift, !down);
                    usb_send_amiga_key(amigaCursorRight, !down);
                    didSpecialStuff = 1;
                    break;
                case 0x3a: // Caps lock
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
        default:
            break;
    }
    return didSpecialStuff; 
}

int getKeycodesLength(enum ServerType serverType) {
    switch(serverType) {
        case Linux: return sizeof(linux_keycodes);
        case Mac: return sizeof(mac_keycodes);
        case Windows: return sizeof(windows_keycodes);
        case UnknownType: return 0;
    }
}

uint8_t *getKeycodes(enum ServerType serverType) {
    switch(serverType) {
        case Linux: return linux_keycodes;
        case Mac: return mac_keycodes;
        case Windows: return windows_keycodes;
        case UnknownType: return NULL;
    }
}

char *getServerTypeName(enum ServerType serverType) {
    switch(serverType) {
        case Linux: return "linux";
        case Mac: return "mac";
        case Windows: return "windows";
        case UnknownType: return "unknown";
    }
}

enum ServerType getServerType(const char *serverTypeName) {
   if(0 == strcasecmp("linux", serverTypeName)) return Linux;
   if(0 == strcasecmp("mac", serverTypeName)) return Mac;
   if(0 == strcasecmp("windows", serverTypeName)) return Windows;
   else return UnknownType;
}

void s_keyboard(uSynergyCookie cookie, uint16_t key, uint16_t modifiers, uSynergyBool down,
                uSynergyBool repeat)
{
    if(debugLevel > 1)
        fprintf(stderr, "keyboard - incoming, key=0x%04x modifiers=0x%04x down=%d\n", key, modifiers, down);

    if (!repeat) {
        if(!doSpecialKeyActions(key, modifiers, down)) {
            if (key < getKeycodesLength(serverType)) {
                uint8_t amigaKey = keycodes[key];
                if (amigaKey != 0xff) {
                    usb_send_amiga_key(amigaKey, !down);
                }
                else {
                    if (debugLevel > 1) fprintf(stderr, "keyboard - ignored, key=0x%04x modifiers=0x%04x down=%d\n", key, modifiers, down);
                }
            }
            else {
                if (debugLevel) fprintf(stderr, "keyboard - unmapped, key=0x%04x modifiers=0x%04x down=%d\n", key, modifiers, down);
            }
        }
    }
}


void s_joystick(uSynergyCookie cookie, uint8_t joyNum, uint16_t buttons,
                int8_t leftStickX, int8_t leftStickY, int8_t rightStickX,
                int8_t rightStickY)
{
    if (debugLevel) fprintf(stderr, "joystick, num=%u buttons=0x%04x leftStick(x,y)=%d,%d rightStick(x,y)=%d,%d\n",
            joyNum, buttons, leftStickX, leftStickY, rightStickX, rightStickY);
}

void s_clipboard(uSynergyCookie cookie, enum uSynergyClipboardFormat format,
                 const uint8_t * data, uint32_t size)
{
    if (debugLevel) fprintf(stderr, "clipboard, size=%d\n", size);
}

void usb_cleanup() {
    libusb_exit(NULL);
}

int main(int argc, char **argv)
{
    uSynergyContext context;
    char *synergyClientName = "amiga";
    int keyCodesFromStdin = 0;
    int listUsbDevices = 1;
    int option = 0;

    while ((option = getopt(argc, argv,"n:t:s:x:y:d:h")) != -1) {
        switch (option) {
            case 'n':
                synergyClientName = optarg; 
                break;
            case 't':
                serverType = getServerType(optarg);
                break;
            case 's':
                usbSerialNum = optarg;
                break;
            case 'x':
                amigaMouseResWidth = atoi(optarg);
                break;
            case 'y':
                amigaMouseResHeight = atoi(optarg);
                break;
            case 'd':
                debugLevel = atoi(optarg);
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-n synergyClientName] [-t synergyServerType linux/mac/windows] [-s usbSerialNum] [-x amigaMouseResolutionWidth] [-y amigaMouseResulutionHeight] [-d debugLevel] [-]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (optind < argc && strcmp(argv[optind], "-") == 0) {
        keyCodesFromStdin = 1;
    }

    int libusbResult = libusb_init(NULL);
    if(0 != libusbResult) {
        fprintf(stderr, "Initializing libusb: %s\n", libusb_strerror(libusbResult));
    }
    atexit(usb_cleanup);
    if(listUsbDevices)
        listUsbDeviceMatches(VENDOR_ID, PRODUCT_ID);
    usbhandle = find_device(VENDOR_ID, PRODUCT_ID, usbSerialNum);

    if (keyCodesFromStdin) {
        int c;
        fprintf(stderr, "Will send stdin as raw amiga keycodes to the avr\n");
        while((c = getchar())) {
            if (c < 0 || c == EOF) return 0;
            usb_send_amiga_key(c, c & 0x80);
        }
    }
    
    if(UnknownType == serverType) {
        fprintf(stderr, "Unknown server type!\n");
        return 1;
    }

    keycodes = getKeycodes(serverType);
    fprintf(stderr, "Server type: %s\n", getServerTypeName(serverType));

    fprintf(stderr, "Amiga mouse resolution: %dx%d\n", amigaMouseResWidth, amigaMouseResHeight);

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
    context.m_clientWidth = amigaMouseResWidth;
    context.m_clientHeight = amigaMouseResHeight;

    for (;;) {
        uSynergyUpdate(&context);
    }
}

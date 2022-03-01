//

#include <system.h>

// https://github.com/klange/toaruos/blob/c4df295848fe43fb352989e3d62248c918e8542a/modules/ps2mouse.c
// Keyboard/Mouse Hardware Ports
//0x60    Read/Write  Data Port
//0x64    Read    Status Register
//0x64    Write   Command Register

#define PS2_DATA                 	0x60
#define PS2_STATUS               	0x64
#define PS2_CMD                     0x64
#define SCANCODE_MASK_RELEASED   	0x80
#define PS2_CONFIG_READ             0x20
#define PS2_CONFIG_WRITE        	0x60
#define PS2_ACK_BYTE                0xFA
#define MOUSE_WRITE              	0xD4

#define PS2_1_DISABLE 0xAD
#define PS2_2_DISABLE 0xA7
#define PS2_1_ENABLE  0xAE
#define PS2_2_ENABLE  0xA8
#define PS2_1_TEST    0xAB
#define PS2_2_TEST    0xA9

#define PS2_DEVICE_IDENTIFY             0xF2 // The mouse stops sending automatic packets.
#define PS2_DEVICE_ENABLE_STREAMING     0xF4 // The mouse starts sending automatic packets when the mouse moves or is clicked.
#define PS2_DEVICE_DISABLE_STREAMING    0xF5 // The mouse stops sending automatic packets.
#define PS2_DEVICE_RESET                0xFF // The mouse probably sends ACK (0xFA) plus several more bytes, then resets itself, and always sends 0xAA.

// Mouse 3-byte Set

#define MOUSE_DEFAULT            	0x00 //
#define MOUSE_SCROLLWHEEL        	0x01 //
#define MOUSE_BUTTONS            	0x02 //
#define MOUSE_F_BIT              	0x20 //
#define MOUSE_V_BIT              	0x08 //
#define MOUSE_CMD_RESEND            0xFE // This command makes the mouse send its most recent packet to the host again.
#define MOUSE_CMD_S
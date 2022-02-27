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
#define SCANCODE_MASK_RELEASED   	
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
#define MOUSE_CMD_SET_DEFAULTS      0xF6 // Disables streaming, sets the packet rate to 100 per second, and resolution to 4 pixels per mm.
#define MOUSE_CMD_ENABLE_STREAMING  0xF4 // The mouse starts sending automatic packets when the mouse moves or is clicked.
#define MOUSE_CMD_SET_SAMPLE_RATE   0xF3 // Requires an additional data byte: automatic packets per second (see below for legal values).
#define MOUSE_CMD_GET_MOUSE_ID      0xF2 // The mouse sends sends its current "ID", which may change with mouse initialization.
#define MOUSE_CMD_PACKET_REQ        0xEB // The mouse sends ACK, followed by a complete mouse packet with current data.
#define MOUSE_CMD_STATUS_REQ        0xE9 // The mouse sends ACK, then 3 status bytes. See below for the status byte format.
#define MOUSE_CMD_RESOLUTION        0xE8 // Requires an additional data byte: pixels per millimeter resolution (value 0 to 3)

// UNUSED !!
#define SET_REMOTE_MODE				0xF0
#define SET_WRAP_MODE 				0xEE
#define RESET_WRAP_MODE				0xEC
#define SET_STREAM_MODE				0xEA

////////////////////////////////////////////////////////////////
// currently allow up to 255 characters to be buffered for use

// TODO: should scan codes buffer as u16 ?? might want to store flags?
// maybe u8 IRQ scan buf and u32 kernel scan buf
#define MAX_BUFFERED_INPUT_KEYS 255
// fixed-size ring buff using overflow to wrap around
u8 kb_buf[MAX_BUFFERED_INPUT_KEYS] = { 0, };
u8 kb_buf_first = 0; // points to first scan (not read yet)
u8 kb_buf_last = 0; // points to last scan received (LIFO)
b32 kb_buf_empty() { return kb_buf_last == kb_buf_first; }

//////////////////////////////////////////////////////////////////
// Scan->Print

// http://www.quadibloc.com/comp/scan.htm
// https://www.vmware.com/support/ws55/doc/ws_devices_keymap_vscan.html

//
u8 scan_to_ascii_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	  /* 9 */
    '9', '0', '-', '=', '\b', /* Backspace */
    '\t', /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key (28) */
    KBDUS_CONTROL, /* 29 - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
    '\'', '`', KBDUS_LEFTSHIFT, /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */
    'm', ',', '.', '/',   KBDUS_RIGHTSHIFT, /* Right shift */
    '*', KBDUS_ALT, /* Alt */
    ' ', /* Space bar */
    KBDUS_CAPSLOCK, /* Caps lock */
    KBDUS_F1, KBDUS_F2, KBDUS_F3, KBDUS_F4, KBDUS_F5,
    KBDUS_F6, KBDUS_F7, KBDUS_F8, KBDUS_F9, KBDUS_F10, /* < ... F10 */
    KBDUS_NUMLOCK, /* 69 - Num lock*/
    KBDUS_SCROLLLOCK, /* Scroll Lock */
    KBDUS_HOME, /* Home key */
    KBDUS_UPARROW, /* Up Arrow */
    KBDUS_PAGEUP, /* Page Up */
    '-',
    KBDUS_LEFTARROW, /* Left Arrow */
    0,
    KBDUS_RIGHTARROW, /* Right Arrow */
    '+',
    KBDUS_END, /* 79 - End key*/
    KBDUS_DOWNARROW, /* Down Arrow */
    KBDUS_PAGEDOWN, /* Page Down */
    KBDUS_INSERT, /* Insert Key */
    KBDUS_DELETE, /* Delete Key */
    0,   0,   0,
    KBDUS_F11, KBDUS_F12,
    0, /* All other keys are undefined */
};

//////////////////////////////////////////////////////////////////

// TODO: convert into ring buffer impl
/// Reads next character of the input stream
/// returns 0 if no key in buffer;
kbscan_t keyboard_read_next()
{
    // TODO: support string formatting in serial_write
    while(kb_buf_first == kb_buf_last) {
        return 0;
    }
    return kb_buf[kb_buf_first++];
}

typedef enum {
    SUCCESS,
    ERROR_TIMEOUT = -1,
    ERROR_UNKNOWN = -2,
    ERROR_GENERIC = -3
} RETURN_CODE;

/// All output must wait for STATUS bit 1 to become clear.
static inline RETURN_CODE ps2_wait_write()
{
    u32 timeout = 100000;
    while(--timeout) {                 //
        if(! (inb(PS2_STATUS) & 0x02)) {
            return SUCCESS;
        }
    }
    trace("PS2 Timed Out!\n");
    return ERROR_TIMEOUT;
}

/// bytes cannot be read until STATUS bit 0 is set
static inline RETURN_CODE ps2_wait_read()
{
    u32 timeout = 100000;
    while(--timeout) {
        if((inb(PS2_STATUS) & 0x01)) {
            return SUCCESS;
        }

    }
    trace("PS2 Timed Out!\n");
    return ERROR_TIMEOUT;
}

// Keyboard Hardware Interrupt
// Note: we don't need to check status/avail bit with interrupts
void keyboard_handler(isr_stack_state *r)
{
    UNUSED_PARAM(r);
    //kserialf("+\n");

    //    // TODO: why does this fail? maybe reading the port itself makes it work fine?
    //    ps2_wait_read();

    u8 scancode = inb(PS2_DATA);

    // TODO: real input event system
    // - events for down, up, pressed, repeated
    // - look at how GLFW, SMFL, SDL handles this

    if (scancode & SCANCODE_MASK_RELEASED)
    {
    }
    else
    {
        kb_buf[kb_buf_last++] = scancode;

        enum {
            PRINT_MODE_ASCII,
            PRINT_MODE_SCAN
        };
        //u8 _curPrintMode = PRINT_MODE_ASCII;
        u8 _curPrintMode = PRINT_MODE_SCAN;
        if(scancode == SCAN_US_ENTER) {
            kputs("Elasped Time (in seconds): ");
            printInt( timer_seconds() );
            kputch('\n');
        } else if(scancode == SCAN_US_F2) {
            trace("\nPressed F2!\n");
            jump_usermode();
        } else if(scancode == SCAN_US_F3) {
            trace("\nPressed F3!\n");
            print_irq_counts();
        } else if(scancode == SCAN_US_F4) {
            trace("\nPressed F4!\n");
            print_blocks_avail();
        } else if(_curPrintMode == PRINT_MODE_SCAN) {
            printHex(scancode);
            kputch('[');
            kputch(scan_to_ascii_us[scancode]);
            kputch(']');
        } else {
            kputch(scan_to_ascii_us[scancode]);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// Mouse
//
// http://wiki.osdev.org/Mouse_Input

internal mouse_device_packet mouse_packets[256];
internal u8 mouse_packets_first = 0;
internal u8 mouse_packets_last = 0;

internal u8 mouse_mode = 0;
internal u8 mouse_cycle = 0;
internal i32 mouse_x = 0;
internal i32 mouse_y = 0;
internal u16 mouse_buttons = 0;

//TODO: i think 4 would be enough??
internal u8 mouse_byte[5] = { 0, };

// TODO: Need to think about coordinate system
i32 mouse_get_x() { return mouse_x; }
i32 mouse_get_y() { return mouse_y; }
u16 mouse_get_buttons() { return mouse_buttons; }
i8 mouse_get_scrolling() { return 0; }

void add_packet(mouse_device_packet packet) {
    mouse_packets[mouse_packets_last++] = packet;
}

// TODO: need real event system (pipes, socket, slot/signal, messages)
mouse_device_packet read_next_packet() {
    mouse_device_packet p;
    if(mouse_packets_first < mouse_packets_last)
        p = mouse_packets[mouse_packets_first++];
    return p;
}

void mouse_handler(isr_stack_state *r)
{
    UNUSED_PARAM(r);

    u8 mouse_in = inb(PS2_DATA);

    switch (mouse_cycle) {
        case 0:
            mouse_byte[0] = mouse_in;
            if (!(mouse_in & 0x08)) {
                goto read_next;
            }
            ++mouse_cycle;
            break;
        case 1:
            mouse_byte[1] = mouse_in;
            ++mouse_cycle;
            break;
        case 2:
            mouse_byte[2] = mouse_in;
            if (mouse_mode == MOUSE_SCROLLWHEEL || mouse_mode == MOUSE_BUTTONS) {
                ++mouse_cycle;
                break;
            }
            goto finish_packet;
        case 3:
            mouse_byte[3] = mouse_in;
            goto finish_packet;
    }

    goto read_next;

finish_packet:

    mouse_cycle = 0;
    u8 state = mouse_byte[0];
    if (state & 0x80 || state & 0x40) {
        /* x/y overflow? bad packet! */
        goto read_next;
    }

    mouse_device_packet packet;
    packet.magic = MOUSE_MAGIC;
    packet.x_difference = mouse_byte[1] - ((state << 4) & 0x100);
    packet.y_difference = -(mouse_byte[2] - ((state << 3) & 0x100));
    packet.buttons = 0;

    if (mouse_byte[0] & 0x01) {
        packet.buttons |= LEFT_CLICK;
    }

    if (mouse_byte[0] & 0x02) {
        packet.buttons |= RIGHT_CLICK;
    }

    if (mouse_byte[0] & 0x04) {
        packet.buttons |= MIDDLE_CLICK;
    }

    if (mouse_mode == MOUSE_SCROLLWHEEL && mouse_byte[3]) {
        if (mouse_byte[3] > 0) {
            packet.buttons |= MOUSE_SCROLL_DOWN;
        } else if (mouse_byte[3] < 0) {
            packet.buttons |= MOUSE_SCROLL_UP;
        }
    }

    mouse_x = CLAMP(mouse_x + packet.x_difference, 0, 600);
    mouse_y = CLAMP(mouse_y + packet.y_difference, 0, 400);

    //mouse_y += packet.y_difference;

    mouse_buttons = packet.buttons;

    add_packet(packet);

read_next:
    inb(PS2_STATUS);

    return;
}

static inline void mouse_write(u8 data)
{
    outb(PS2_CMD, MOUSE_WRITE);
    if(ps2_wait_write() == ERROR_TIMEOUT)
        return;
    outb(PS2_DATA, data);
}

/// Get's response from mouse
internal inline u8 mouse_read()
{
    if(ps2_wait_read() == ERROR_TIMEOUT)
        return 0; // no data
    return inb(PS2_DATA);
}

// TODO: combine with keyboard_install
/// Install Mouse IRQ Handler
void ps2_install()
{
    //    trace("[INFO] Not Installing Mouse!\n");
    //    return;
    trace("Installing Mouse PS/2\n");

    u8 status, result;

    // disable PS2 ports
    outb(PS2_CMD, PS2_1_DISABLE);
    outb(PS2_CMD, PS2_2_DISABLE);

    // Do a couple dummy reads from the data port.
    inb(PS2_DATA);
    inb(PS2_DATA);
    inb(PS2_DATA);

    // http://wiki.osdev.org/%228042%22_PS/2_Controller#First_PS.2F2_Port
    // 1: Initialise USB Controllers
    // 2: Determine if the PS/2 Controller Exists

    ps2_wait_write();

    // read config and set bit 2
    outb(PS2_CMD, PS2_CONFIG
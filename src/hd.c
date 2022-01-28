
#include <system.h>

inline void ata_delay400ns()
{
    inb(HD_ST); inb(HD_ST); inb(HD_ST); inb(HD_ST);
}

void ata_wait_busy() {
	u8 status;
	int timer = 10000;
	while(1) {
		status = inb(HD_ST_ALT);
		if ( !(status & HD_ST_BSY) ) {
			break;
		}
		if ( (status & HD_ST_ERR) ) {
			trace("Error in ata_wait_drq!\n");
		}
		if (--timer < 0) {
			trace("Timeout in ata_wait_busy!\n");
			break;
		}
	}
}

void ata_wait_drq() {
	u8 status;
	int timer = 10000;
	while(1) {
		status = inb(HD_ST_ALT);
		if ( !(status & HD_ST_DRQ) ) {
			break;
		}
		if ( (status & HD_ST_ERR) ) {
			trace(" Error in ata_wait_drq! ");
		}
		if (--timer < 0) {
			trace(" Timeout in ata_wait_drq! ");
			break;
		}
	}
}

void ata_wait_ready() {
	u8 status;
	int timer = 10000;
	while(1) {
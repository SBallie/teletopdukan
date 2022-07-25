
#include <system.h>

///////////////////////////////////////////////////////////////////

#define ticks_from_s(x)  	(x * _timer_hz)
#define ticks_from_ms(x) 	((x * _timer_hz) / 1000)
#define ticks_to_s(x)		(x / _timer_hz)
#define ticks_to_ms(x)		((x * 1000) / _timer_hz)

#define PIT_PORT    0x40
#define PIT_CTRL    0x43
#define PIT_CMD_SET 0x36
#define PIT_MASK    0xFF

// Our desired timer properties
// Default interrupt freq 18.222Hz
static u32 _timer_hz = 100;
static u32 _timer_ticks = 0;
// static u32 _secondsFromBoot = 0;
// static u32 _timer_subticks = 0;
// static i32 _timer_drift = 0;

// Scheduling Tasks
static u32 _next_preempt = 0;
static u32 _ticks_per_schedule = 50;

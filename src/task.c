
//
//  task.cpp
//  dummy
//
//  Created by Steve Tranby on 3/2/16.
//  Copyright © 2016 Steve Tranby. All rights reserved.
//

//#include <stdint.h>
#include <system.h>

// TODO: alloc on heap instead (no variable)
internal Task* runningTask;
i32 getpid() { return runningTask->pid; }

#define MAX_PROCESSES 10

// TODO: confirm zeroes out, but also should use kmalloc instead of hardcode
internal Task processes[MAX_PROCESSES];
internal u8 processesCount = 0;
internal u8 curProcessIndex = 0;

//////////////////////////////////////////////////////////////////
// Testing

static void main_A()
{
    trace("Starting Process A!\n");
    for(;;)
    {
        //trace("Running Process A!\n");
        //delay_ms(100);
        k_preempt();
    }
}

static void main_B()
{
    trace("Starting Process B!\n");
    for(;;)
    {
        //trace("Running Process B!\n");
        //delay_ms(200);
        k_preempt();
    }
}

static void main_C()
{
    trace("Starting Process C!\n");
    for(;;)
    {
        //trace("Running Process C!\n");
        //delay_ms(300);
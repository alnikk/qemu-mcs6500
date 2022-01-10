#include "qemu/osdep.h"
#include "helper.h"

bool mcs6500_cpu_exec_interrupt(CPUState *cs, int interrupt_request)
{
    return false;
}

void mcs6500_cpu_do_interrupt(CPUState *cs)
{
}

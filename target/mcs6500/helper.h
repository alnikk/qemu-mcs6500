#ifndef MCS6500_HELPER_H
#define MCS6500_HELPER_H

#include "cpu.h"

bool mcs6500_cpu_exec_interrupt(CPUState *cs, int interrupt_request);
void mcs6500_cpu_do_interrupt(CPUState *cs);

#endif // MCS6500_HELPER_H

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "cpu.h"
#include "exec/exec-all.h"

void restore_state_to_opc(CPUMCS6500State *env, TranslationBlock *tb,
                          target_ulong *data)
{
    env->pc = (uint16_t) (data[0] & PC_MASK);
}

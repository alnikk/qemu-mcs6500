/*
 *  MOS65xx emulation
 *
 *  Copyright (c) 2022 Alexandre Guyon
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/qemu-print.h"
#include "exec/exec-all.h"
#include "cpu.h"
#include "hw/core/tcg-cpu-ops.h"
#include "exec/helper-proto.h"
#include "sysemu/runstate.h"
#include "qemu/main-loop.h"

bool mos65xx_cpu_exec_interrupt(CPUState *cs, int interrupt_request)
{
    bool ret = false;
    CPUClass *cc = CPU_GET_CLASS(cs);
    MOS65XXCPU *cpu = MOS65XX_CPU(cs);
    CPUMOS65XXState *env = &cpu->env;

    if (interrupt_request & CPU_INTERRUPT_RESET) {
        cs->exception_index = EXCP_RESET;
        cc->tcg_ops->do_interrupt(cs);
        cs->interrupt_request &= ~CPU_INTERRUPT_RESET;
        ret = true;
    }
    if (interrupt_request & CPU_INTERRUPT_HARD) {
        if (env->excp & EXCP_NMI) {
            cs->exception_index = EXCP_NMI;
            cc->tcg_ops->do_interrupt(cs);
            ret = true;
            env->excp &= ~EXCP_NMI;
            cs->interrupt_request &= ~CPU_INTERRUPT_HARD;
        }
        if (env->excp & EXCP_IRQ && cpu_interrupts_enabled(env)) {
            cs->exception_index = EXCP_IRQ;
            cc->tcg_ops->do_interrupt(cs);
            ret = true;
            env->excp &= ~EXCP_IRQ;
            cs->interrupt_request &= ~CPU_INTERRUPT_HARD;
        }
    }

    return ret;
}

void mos65xx_cpu_do_interrupt(CPUState *cs)
{
    MOS65XXCPU *cpu = MOS65XX_CPU(cs);
    CPUMOS65XXState *env = &cpu->env;
    uint16_t vector_addr;
    uint16_t vector;
    MemTxResult txres;

    if (cs->exception_index == EXCP_RESET) {
        vector_addr = VECTOR_ADDR_RESET;
    } else if (cs->exception_index == EXCP_NMI) {
        vector_addr = VECTOR_ADDR_NMI;
    } else if (cs->exception_index == EXCP_IRQ) {
        vector_addr = VECTOR_ADDR_IRQ;
    } else {
        qemu_printf("handling %d exception is not supported\n", cs->exception_index);
        return;
    }

    /* Get vector from address space */
    vector = address_space_lduw_le(cs->as, vector_addr, MEMTXATTRS_UNSPECIFIED, &txres);
    if (txres != MEMTX_OK) {
        qemu_printf("mos65xx error handling interrupt vector\n");
        return;
    }

    /* decrease sp ; store pc ; pc to vector */
    cpu_stb_data(env, env->sp--, (env->pc & 0x00ff00) >> 8);
    cpu_stb_data(env, env->sp--, (env->pc & 0x0000ff));
    env->pc = vector;

    cs->exception_index = -1;
}

hwaddr mos65xx_cpu_get_phys_page_debug(CPUState *cs, vaddr addr)
{
    return addr;
}

int mos65xx_cpu_memory_rw_debug(CPUState *cs, vaddr addr, uint8_t *buf,
                            int len, bool is_write)
{
    return cpu_memory_rw_debug(cs, addr, buf, len, is_write);
}

bool mos65xx_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                      MMUAccessType access_type, int mmu_idx,
                      bool probe, uintptr_t retaddr)
{
    /* access to memory. nothing special */
    hwaddr paddr = address;
    int prot = PAGE_READ | PAGE_WRITE | PAGE_EXEC;
    MemTxAttrs attrs = {};

    tlb_set_page_with_attrs(cs, address, paddr, attrs, prot,
                            mmu_idx, TARGET_PAGE_SIZE);

    return true;
}

void helper_unsupported(CPUMOS65XXState *env)
{
    CPUState *cs = env_cpu(env);

    qemu_log("unsupported\n");
    cpu_loop_exit(cs);
}

void helper_break(CPUMOS65XXState *env)
{
    CPUState *cs = env_cpu(env);

    qemu_mutex_lock_iothread();
    cs->exception_index = EXCP_DEBUG;
    qemu_mutex_unlock_iothread();

    cpu_loop_exit(cs);
}

void helper_quit(CPUMOS65XXState *env)
{
    CPUState *cs = env_cpu(env);

    qemu_mutex_lock_iothread();
    qatomic_set(&cs->exit_request, true);
    cs->exception_index = EXCP_HALTED;
    qemu_mutex_unlock_iothread();
    qemu_system_shutdown_request(SHUTDOWN_CAUSE_GUEST_SHUTDOWN);
    /* wait all thread being executed */
    sleep(1);

    {
        FILE *dump = fopen ("core_dump.pout", "w+");
        cpu_dump_state(cs, dump, 0);
        fclose(dump);
    }
    cpu_loop_exit(cs);
}

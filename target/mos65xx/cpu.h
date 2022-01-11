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

#ifndef MOS65XX_CPU_H
#define MOS65XX_CPU_H

#include "cpu-qom.h"
#include "exec/cpu-defs.h"

/* We don't support user mode */
#define TCG_GUEST_DEFAULT_MO 0

#ifdef CONFIG_USER_ONLY
#error "MOS 65xx 8-bit does not support user mode"
#endif

#define CPU_RESOLVING_TYPE TYPE_MOS65XX_CPU

#define SR_C 0 /* Carry */
#define SR_Z 1 /* Zero result */
#define SR_I 2 /* Interrupt disable */
#define SR_D 3 /* Decimal */
#define SR_B 4 /* Break */
#define SR_E 5 /* Expension - not used */
#define SR_V 6 /* Overflow */
#define SR_N 7 /* Negative result */

#define PC_MASK 0xFFFF

#define EXCP_RESET 1 << 0
#define EXCP_IRQ   1 << 1
#define EXCP_NMI   1 << 2

#define VECTOR_ADDR_RESET 0xFFFC
#define VECTOR_ADDR_IRQ 0xFFFE
#define VECTOR_ADDR_NMI 0xFFFA

typedef struct CPUMOS65XXState CPUMOS65XXState;
struct CPUMOS65XXState {
    uint16_t pc;
    uint8_t sr;
    uint8_t sp;
    uint8_t x;
    uint8_t y;
    uint8_t acc;

    uint8_t excp;
    uint8_t nIRQ;
};

typedef struct MOS65XXCPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/

    CPUNegativeOffsetState neg;
    CPUMOS65XXState env;
} MOS65XXCPU;

void mos65xx_cpu_do_interrupt(CPUState *cpu);
bool mos65xx_cpu_exec_interrupt(CPUState *cpu, int int_req);
hwaddr mos65xx_cpu_get_phys_page_debug(CPUState *cpu, vaddr addr);

#define MMU_NOMMU_IDX   0

#define cpu_list mos65xx_cpu_list
#define cpu_mmu_index(env, index) MMU_NOMMU_IDX

void mos65xx_cpu_tcg_init(void);

void mos65xx_cpu_list(void);
int cpu_mos65xx_exec(CPUState *cpu);
int mos65xx_cpu_memory_rw_debug(CPUState *cs, vaddr address, uint8_t *buf,
                                int len, bool is_write);

int mos65xx_cpu_gdb_read_register(CPUState *cpu, GByteArray *buf, int reg);
int mos65xx_cpu_gdb_write_register(CPUState *cpu, uint8_t *buf, int reg);


static inline void cpu_get_tb_cpu_state(CPUMOS65XXState *env, target_ulong *pc,
                                        target_ulong *cs_base, uint32_t *pflags)
{
    *pc = env->pc;
    *cs_base = 0;
    *pflags = 0;
}

static inline int cpu_interrupts_enabled(CPUMOS65XXState *env)
{
    return !((env->sr >> SR_I) & 0x1);
}

bool mos65xx_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                      MMUAccessType access_type, int mmu_idx,
                      bool probe, uintptr_t retaddr);

typedef CPUMOS65XXState CPUArchState;
typedef MOS65XXCPU ArchCPU;

#include "exec/cpu-all.h"

#endif // MOS65XX_CPU_H

/*
 *  MCS6500 emulation
 *
 *  Copyright (c) 2020 Alexandre Guyon
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

#ifndef MCS6500_CPU_H
#define MCS6500_CPU_H

#include "hw/core/cpu.h"
#include "exec/cpu-defs.h"

#define TYPE_MCS6500_CPU "6500"
#define CPU_RESOLVING_TYPE TYPE_MCS6500_CPU

#define SR_C 0 /* Carry */
#define SR_Z 1 /* Zero result */
#define SR_I 2 /* Interrupt disable */
#define SR_D 3 /* Decimal */
#define SR_B 4 /* Break */
#define SR_E 5 /* Expension - not used */
#define SR_V 6 /* Overflow */
#define SR_N 7 /* Negative result */

typedef struct CPUMCS6500State CPUMCS6500State;
struct CPUMCS6500State {
    uint16_t pc;
    uint8_t sr;
    uint8_t sp;
    uint8_t x;
    uint8_t y;
    uint8_t acc;
};


#define MCS6500_CPU_CLASS(klass) \
    OBJECT_CLASS_CHECK(MCS6500Class, (klass), TYPE_MCS6500_CPU)
#define MCS6500_CPU(obj) \
    OBJECT_CHECK(MCS6500CPU, (obj), TYPE_MCS6500_CPU)
#define MCS6500_CPU_GET_CLASS(obj) \
    OBJECT_GET_CLASS(MCS6500Class, (obj), TYPE_MCS6500_CPU)

/**
 * MCS6500CPUClass:
 * @parent_realize: The parent class' realize handler.
 * @parent_reset: The parent class' reset handler.
 *
 * A MCS6500 CPU model.
 */
typedef struct MCS6500CPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/
    DeviceRealize parent_realize;
    DeviceReset parent_reset;
} MCS6500CPUClass;

/**
 * MCS6500CPU:
 * @env: #CPUMCS6500State
 *
 * A MCS6500 CPU.
 */
typedef struct MCS6500CPU {
    /*< private >*/
    CPUState parent_obj;
    /*< public >*/
    CPUNegativeOffsetState neg;
    CPUMCS6500State env;
} MCS6500CPU;

typedef CPUMCS6500State CPUArchState;
typedef MCS6500CPU ArchCPU;

#define MMU_USER_IDX 0
#define MMU_NOMMU 0

static inline int cpu_mmu_index(CPUMCS6500State *env, bool ifetch)
{
    return MMU_NOMMU;
}

#include "exec/cpu-all.h"

#endif // MCS6500_CPU_H

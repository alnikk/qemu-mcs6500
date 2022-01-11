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
#include "exec/gdbstub.h"

int mos65xx_cpu_gdb_read_register(CPUState *cs, GByteArray *mem_buf, int n)
{
    MOS65XXCPU *cpu = MOS65XX_CPU(cs);
    CPUMOS65XXState *env = &cpu->env;

    switch(n) {
        case 1:
            gdb_get_reg16(mem_buf, env->pc);
            break;
        case 2:
            gdb_get_reg16(mem_buf, env->sp);
            break;
        case 3:
            gdb_get_reg8(mem_buf, env->sr);
            break;
        case 4:
            gdb_get_reg8(mem_buf, env->x);
            break;
        case 5:
            gdb_get_reg8(mem_buf, env->y);
            break;
        case 6:
            gdb_get_reg8(mem_buf, env->acc);
            break;
        default:
            return -1;
    }

    return 0;
}

int mos65xx_cpu_gdb_write_register(CPUState *cs, uint8_t *mem_buf, int n)
{
    MOS65XXCPU *cpu = MOS65XX_CPU(cs);
    CPUMOS65XXState *env = &cpu->env;

    switch(n) {
        case 1:
            env->pc = *mem_buf;
            break;
        case 2:
            env->sp = *mem_buf;
            break;
        case 3:
            env->sr = *mem_buf;
            break;
        case 4:
            env->x = *mem_buf;
            break;
        case 5:
            env->y = *mem_buf;
            break;
        case 6:
            env->acc = *mem_buf;
            break;
        default:
            return -1;
    }

    return 0;
}

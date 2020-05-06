/*
 * QEMU 6500 Family CPU
 *
 * Copyright (c) 2012 SUSE LINUX Products GmbH
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
 * License along with this library; if not, see
 * <http://www.gnu.org/licenses/lgpl-2.1.html>
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "cpu.h"

static bool mcs6500_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                       MMUAccessType qemu_access_type, int mmu_idx,
                       bool probe, uintptr_t retaddr)
{
    return true;
}

static void mcs6500_cpu_reset(DeviceState *dev)
{
    CPUState *s = CPU(dev);
    MCS6500CPU *cpu = MCS6500_CPU(s);
    MCS6500CPUClass *mcc = MCS6500_CPU_GET_CLASS(cpu);
    CPUMCS6500State *env = &cpu->env;

    mcc->parent_reset(dev);

    memset(env, 0, sizeof(CPUMCS6500State));

    /* TODO: We should set PC from the interrupt vector.  */
    env->pc = 0xFFFF;
}

static void mcs6500_cpu_realizefn(DeviceState *dev, Error **errp)
{
    CPUState *cs = CPU(dev);
    MCS6500CPU *cpu = MCS6500_CPU(cs);
    MCS6500CPUClass *mcc = MCS6500_CPU_GET_CLASS(cpu);
    Error *local_err = NULL;

    cpu_exec_realizefn(cs, &local_err);
    if (local_err != NULL) {
        error_propagate(errp, local_err);
        return;
    }

    cpu_reset(cs);
    qemu_init_vcpu(cs);

    mcc->parent_realize(dev, errp);
}

static void mcs6500_cpu_initfn(Object *obj)
{
    MCS6500CPU *cpu = MCS6500_CPU(obj);

    cpu_set_cpustate_pointers(cpu);
}

static void mcs6500_cpu_class_init(ObjectClass *c, void *data)
{
    MCS6500CPUClass *mcc = MCS6500_CPU_CLASS(c);
    CPUClass *cc = CPU_CLASS(c);
    DeviceClass *dc = DEVICE_CLASS(c);

    device_class_set_parent_realize(dc, mcs6500_cpu_realizefn,
                                    &mcc->parent_realize);
    device_class_set_parent_reset(dc, mcs6500_cpu_reset, &mcc->parent_reset);

    cc->tlb_fill = mcs6500_cpu_tlb_fill;
    cc->tcg_initialize = mcs6500_tcg_init;
}

static const TypeInfo mcs6500_cpus_type_infos[] = {
    { /* base class should be registered first */
        .name = TYPE_MCS6500_CPU,
        .parent = TYPE_CPU,
        .instance_size = sizeof(MCS6500CPU),
        .instance_init = mcs6500_cpu_initfn,
        .class_size = sizeof(MCS6500CPUClass),
        .class_init = mcs6500_cpu_class_init,
    }
};

DEFINE_TYPES(mcs6500_cpus_type_infos)

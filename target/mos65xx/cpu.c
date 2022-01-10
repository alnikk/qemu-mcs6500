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

static void mos65xx_cpu_reset(DeviceState *dev)
{
    CPUState *s = CPU(dev);
    MOS65XXCPU *cpu = MOS65XX_CPU(s);
    MOS65XXCPUClass *mcc = MOS65XX_CPU_GET_CLASS(cpu);
    CPUMOS65XXState *env = &cpu->env;

    mcc->parent_reset(dev);

    memset(env, 0, sizeof(CPUMOS65XXState));
}

static void mos65xx_cpu_set_pc(CPUState *cs, vaddr value)
{
    MOS65XXCPU *cpu = MOS65XX_CPU(cs);

    cpu->env.pc = value; /* internally PC points to words */
}

static bool mos65xx_cpu_has_work(CPUState *cs)
{
    return false;
}

static void mos65xx_cpu_dump_state(CPUState *cs, FILE *f, int flags)
{
    MOS65XXCPU *cpu = MOS65XX_CPU(cs);
    CPUMOS65XXState *env = &cpu->env;

    qemu_fprintf(f, "\n");
    qemu_fprintf(f, "PC:    %04x\n", env->pc);
    qemu_fprintf(f, "SP:    %04x\n", env->sp);
    qemu_fprintf(f, "X:     %04x\n", env->x);
    qemu_fprintf(f, "Y:     %04x\n", env->y);
    qemu_fprintf(f, "ACC:   %04x\n", env->acc);
    qemu_fprintf(f, "SR: [ %c %c %c %c %c %c %c %c ]\n",
                 ((env->sr >> SR_C) & 0x1) ? 'C' : '-',
                 ((env->sr >> SR_Z) & 0x1) ? 'Z' : '-',
                 ((env->sr >> SR_I) & 0x1) ? '-' : 'I',
                 ((env->sr >> SR_D) & 0x1) ? 'D' : '-',
                 ((env->sr >> SR_B) & 0x1) ? 'B' : '-',
                 ((env->sr >> SR_E) & 0x1) ? 'E' : '-',
                 ((env->sr >> SR_V) & 0x1) ? 'V' : '-',
                 ((env->sr >> SR_N) & 0x1) ? 'N' : '-');
}

static void mos65xx_cpu_realizefn(DeviceState *dev, Error **errp)
{
    CPUState *cs = CPU(dev);
    MOS65XXCPU *cpu = MOS65XX_CPU(cs);
    MOS65XXCPUClass *mcc = MOS65XX_CPU_GET_CLASS(cpu);
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

static void mos65xx_cpu_set_int(void *opaque, int irq, int level)
{
    MOS65XXCPU *cpu = opaque;
    CPUMOS65XXState *env = &cpu->env;
    CPUState *cs = CPU(cpu);
    uint64_t mask = (1ull << irq);

    if (level) {
        env->excp |= mask;
        cpu_interrupt(cs, CPU_INTERRUPT_HARD);
    } else {
        env->excp &= ~mask;
        if (env->excp == 0) {
            cpu_reset_interrupt(cs, CPU_INTERRUPT_HARD);
        }
    }
}

static void mos65xx_cpu_initfn(Object *obj)
{
    MOS65XXCPU *cpu = MOS65XX_CPU(obj);

    cpu_set_cpustate_pointers(cpu);

    qdev_init_gpio_in_named(DEVICE(cpu), mos65xx_cpu_set_int, "interrupt lines",
            cpu->env.nIRQ);
}

#include "hw/core/tcg-cpu-ops.h"

static const struct TCGCPUOps mos65xx_tcg_ops = {
    .initialize = mos65xx_cpu_tcg_init,
    .cpu_exec_interrupt = mos65xx_cpu_exec_interrupt,
    .do_interrupt = mos65xx_cpu_do_interrupt,
    .tlb_fill = mos65xx_cpu_tlb_fill,
};

#include "hw/core/sysemu-cpu-ops.h"

static const struct SysemuCPUOps mos65xx_sysemu_ops = {
    .get_phys_page_debug = mos65xx_cpu_get_phys_page_debug,
};

static ObjectClass *mos65xx_cpu_class_by_name(const char *cpu_model)
{
    ObjectClass *oc;

    oc = object_class_by_name(cpu_model);
    if (object_class_dynamic_cast(oc, TYPE_MOS65XX_CPU) == NULL ||
        object_class_is_abstract(oc)) {
        oc = NULL;
    }
    return oc;
}

static void mos65xx_cpu_class_init(ObjectClass *c, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(c);
    CPUClass *cc = CPU_CLASS(c);
    MOS65XXCPUClass *mcc = MOS65XX_CPU_CLASS(c);

    device_class_set_parent_realize(dc, mos65xx_cpu_realizefn,
                                    &mcc->parent_realize);
    device_class_set_parent_reset(dc, mos65xx_cpu_reset, &mcc->parent_reset);

    cc->class_by_name = mos65xx_cpu_class_by_name;

    cc->has_work = &mos65xx_cpu_has_work;
    cc->set_pc = &mos65xx_cpu_set_pc;
    cc->dump_state = &mos65xx_cpu_dump_state;
    cc->memory_rw_debug = mos65xx_cpu_memory_rw_debug;
    cc->sysemu_ops = &mos65xx_sysemu_ops;
    cc->tcg_ops = &mos65xx_tcg_ops;
}

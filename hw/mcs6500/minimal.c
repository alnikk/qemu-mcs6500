/*
 * Minimal board to simulate mcs6500
 *
 * Copyright (c) 2020 Alexandre Guyon
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/hw.h"
#include "hw/sysbus.h"
#include "hw/mcs6500/minimal.h"
#include "target/mcs6500/cpu.h"

static void minimal_init(MachineState *machine)
{
}

static void machine_minimal_init(MachineClass *mc)
{
    mc->desc = "Minimal machine for testing purposes";
    mc->init = minimal_init;
    mc->is_default = true;
    mc->default_cpu_type = TYPE_MCS6500_CPU;
}

DEFINE_MACHINE("minimal", machine_minimal_init)

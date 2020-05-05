/*
 * Nitendo Nes emulation
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
#include "hw/mcs6500/nes.h"
#include "qapi/error.h"
#include "exec/address-spaces.h"
#include "sysemu/sysemu.h"
#include "target/mcs6500/cpu.h"
#include "hw/mcs6500/nes-cartridge.h"

static void nes_init(MachineState *machine)
{
    SocMCS6500State *cpu;
    MemoryRegion *mirrors;
    NesCartridgeState *cartridge;
    Error *err = NULL;

    if (bios_name == NULL) {
        error_report("Please provide a -bios <path to nes rom file> argument");
        exit(1);
    }

    cpu = SOC_MCS6500(object_new(TYPE_SOC_MCS6500));
    object_property_add_child(OBJECT(machine), "soc", OBJECT(cpu),
                              &error_abort);
    object_property_set_bool(OBJECT(cpu), true, "realized", &err);
    object_unref(OBJECT(cpu));

    mirrors = g_new0(MemoryRegion, NES_NB_MIRRORS);
    for (uint8_t i = 0 ; i < NES_NB_MIRRORS ; ++i) {
        char name[] = "mem_aliasN";
        snprintf(name, sizeof(name), "mem_alias%d", i);
        memory_region_init_alias(&mirrors[i], OBJECT(machine), name, machine->ram,
                0, NES_RAM_SIZE);
        memory_region_add_subregion(get_system_memory(), nes_get_base_ram_mirror(i),
                &mirrors[i]);
    }

    cartridge = NES_CARTRIDGE(object_new(TYPE_NES_CARTRIDGE));
    cartridge->rom_path = strdup(bios_name);
    object_property_add_child(OBJECT(machine), "cartridge", OBJECT(cartridge),
                            &error_abort);
    object_property_set_bool(OBJECT(cartridge), true, "realized", &err);
    object_unref(OBJECT(cartridge));
}

static void nes_machine_init(MachineClass *mc)
{
    mc->desc = "Nitendo Nes";
    mc->init = nes_init;
    mc->default_cpu_type = TYPE_MCS6500_CPU;
    mc->default_ram_size = NES_RAM_SIZE;
    mc->default_ram_id = "nes.ram";
}

DEFINE_MACHINE("nes", nes_machine_init)

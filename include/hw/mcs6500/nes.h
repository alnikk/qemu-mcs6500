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
 */

#ifndef NES_H
#define NES_H

#include "hw/boards.h"
#include "exec/memory.h"
#include "hw/mcs6500/mcs6500.h"

#define TYPE_NES "nes-machine"
#define NES(obj) OBJECT_CHECK(NesState, (obj), TYPE_NES)

#define NES_RAM_SIZE 0x0800 /* 2 KiB */
#define NES_BASE_RAM_ADDR 0x0
#define NES_NB_MIRRORS 3
#define NES_BASE_RAM1_MIRROR_ADDR 0x0800
#define NES_BASE_RAM2_MIRROR_ADDR 0x1000
#define NES_BASE_RAM3_MIRROR_ADDR 0x1800

static inline int nes_get_base_ram_mirror(int i)
{
    switch (i) {
        case 0:
            return NES_BASE_RAM1_MIRROR_ADDR;
            break;
        case 1:
            return NES_BASE_RAM2_MIRROR_ADDR;
            break;
        case 2:
            return NES_BASE_RAM3_MIRROR_ADDR;
            break;
        default:
            assert(0);
    }
}

#define NES_CARTRIDGE_BASE 0x4020
#define NES_CARTRIDGE_SIZE 0xBFE0 /* Till the end of the memory map (0xFFFF) */

#endif // NES_H

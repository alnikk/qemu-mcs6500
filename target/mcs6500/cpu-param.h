/*
 * MCS6500 cpu parameters for qemu.
 *
 * Copyright (c) 2020 Alexandre Guyon
 * SPDX-License-Identifier: LGPL-2.0+
 */

#ifndef MCS6500_CPU_PARAM_H
#define MCS6500_CPU_PARAM_H

/* Qemu is not capable of handling 8-bit processors we're telling it to handle
 * 32 bits instead. We still have to fake a MMU.
 *
 * MCS6500 are 8 bits processors with a 16 bits address space. There's two pages
 * which can be reffered as MMU page. Still, those pages can be accessed via
 * pointer inderection with different addressing modes.
 */

#define TARGET_LONG_BITS 32 /* Qemu does not handle 8 bits */

/* Following values are deliberately wrong */
#define TARGET_PAGE_BITS 8
#define TARGET_PHYS_ADDR_SPACE_BITS 24
#define TARGET_VIRT_ADDR_SPACE_BITS 24
#define NB_MMU_MODES 1

#endif // MCS6500_CPU_PARAM_H

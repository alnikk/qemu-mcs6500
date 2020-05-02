/*
 * Minimal device to simulate mcs6500
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

#ifndef MCS6500_H
#define MCS6500_H

#include "hw/qdev-core.h"
#include "cpu.h"

#define TYPE_SOC_MCS6500 "mcs6500-soc"
#define SOC_MCS6500(obj) OBJECT_CHECK(SocMCS6500State, (obj), TYPE_SOC_MCS6500)

typedef struct {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    MCS6500CPU *cpu;
} SocMCS6500State;

#endif // MCS6500_H

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
#include "hw/mcs6500/mcs6500.h"

static void mcs6500_init(Object *obj)
{
}

static void mcs6500_realize(DeviceState *dev, Error **errp)
{
    SocMCS6500State *soc = SOC_MCS6500(dev);
    soc->cpu = MCS6500_CPU(cpu_create(TYPE_MCS6500_CPU));
}

static void mcs6500_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = mcs6500_realize;
    dc->desc = "MCS6500 chip";
    dc->user_creatable = false;
}

static const TypeInfo mcs6500_type_info = {
    .name = TYPE_SOC_MCS6500,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(SocMCS6500State),
    .instance_init = mcs6500_init,
    .class_init = mcs6500_class_init,
};

static void mcs6500_register_types(void)
{
    type_register_static(&mcs6500_type_info);
}

type_init(mcs6500_register_types)

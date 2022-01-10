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

#ifndef MOS65XX_CPU_QOM_H
#define MOS65XX_CPU_QOM_H

#include "hw/core/cpu.h"
#include "qom/object.h"

#define TYPE_MOS65XX_CPU "mos65xx-cpu"

OBJECT_DECLARE_TYPE(MOS65XXCPU, MOS65XXCPUClass,
                    MOS65XX_CPU)

/**
 * MOS65XXCPUClass:
 * @parent_realize: The parent class' realize handler.
 * @parent_reset: The parent class' reset handler.
 *
 * A MOS65XX CPU model.
 */
struct MOS65XXCPUClass {
    /*< private >*/
    CPUClass parent_class;
    /*< public >*/
    DeviceRealize parent_realize;
    DeviceReset parent_reset;
};

#endif // MOS65XX_CPU_QOM_H

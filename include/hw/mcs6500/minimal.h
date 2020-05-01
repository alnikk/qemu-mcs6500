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
 */

#ifndef MINIMAL_H
#define MINIMAL_H

#include "hw/boards.h"

typedef struct {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
} MinimalState;

#endif // MINIMAL_H

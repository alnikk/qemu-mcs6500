#ifndef NES_CARTRIDGE_H
#define NES_CARTRIDGE_H

#include "hw/qdev-core.h"
#include "exec/memory.h"

#define TYPE_NES_CARTRIDGE "nescartridge"
#define NES_CARTRIDGE(obj) OBJECT_CHECK(NesCartridgeState, (obj), TYPE_NES_CARTRIDGE)

typedef struct {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    char *rom_path;
    MemoryRegion ram;
    MemoryRegion rom;
} NesCartridgeState;

#endif // NES_CARTRIDGE_H

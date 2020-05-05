#include "qemu/osdep.h"
#include "exec/address-spaces.h"
#include "hw/mcs6500/nes-cartridge.h"
#include "hw/video-games/ines.h"

#include <stdio.h>

static void nes_cartridge_init(Object *obj)
{
    printf("Init'd\n");
}

static void nes_cartridge_realize(DeviceState *dev, Error **errp)
{
    NesCartridgeState *cart = NES_CARTRIDGE(dev);
    int ret;
    int sections_size;
    const struct ines_mapper_section *sections;

    ret = ines_load_file(cart->rom_path);
    if (ret < 0) {
        printf("Cannot load %s\n", cart->rom_path);
        // FIXME (aguyon): hanlde error
        return;
    }

    ret = ines_is_supported();
    if (ret < 0) {
        printf("Rom not supported\n");
        return;
    }

    sections_size = ines_get_sections(&sections);
    if (sections_size < 0) {
        printf("Cannot retrieve sections\n");
        return;
    }

    for (int i = 0 ; i < sections_size ; ++i) {
        void *section;
        size_t section_size;

        switch (sections[i].section_id) {
            case INES_SECTION_PRG_RAM:
                memory_region_init_ram_nomigrate(&cart->ram, OBJECT(cart), "PRG RAM",
                        sections[i].end - sections[i].start, errp);
                memory_region_add_subregion(get_system_memory(),
                        sections[i].start, &cart->ram);
                break;
            case INES_SECTION_PRG_ROM:
                ret = ines_get_section_by_id(sections[i].section_id, &section, &section_size);
                if (ret < 0) {
                    printf("Cannot get section\n");
                    return;
                }
                memory_region_init_ram_ptr(&cart->rom, OBJECT(cart), "PRG ROM", section_size, section);
                memory_region_set_readonly(&cart->rom, true);
                memory_region_add_subregion(get_system_memory(), sections[i].start, &cart->rom);
                break;
            default:
                printf("Cannot handle %d section\n", sections[i].section_id);
                return;
        }
    }

    printf("Realized\n");
}

static void nes_cartridge_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = nes_cartridge_realize;
    dc->desc = "Nes cartridge";
    dc->user_creatable = false;
}

static const TypeInfo nes_cartridge_type_info = {
    .name = TYPE_NES_CARTRIDGE,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(NesCartridgeState),
    .instance_init = nes_cartridge_init,
    .class_init = nes_cartridge_class_init,
};

static void nes_cartridge_register_types(void)
{
    type_register_static(&nes_cartridge_type_info);
}

type_init(nes_cartridge_register_types)

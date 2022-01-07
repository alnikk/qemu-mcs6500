#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "hw/video-games/ines.h"

#define DEBUG
#ifdef DEBUG
#include <stdio.h>

#define debug printf

#define INES_FLAG6_MIRRORING_H_VALUE 0
#define INES_FLAG6_MIRRORING_H_NAME "horizontal mirroring"
#define INES_FLAG6_MIRRORING_V_VALUE 1
#define INES_FLAG6_MIRRORING_V_NAME "vertical mirroring"
#endif // DEBUG

/*
 * iNES format: http://fms.komkon.org/EMUL8/NES.html#LABM
 */

#define INES_MAGIC1 'N'
#define INES_MAGIC2 'E'
#define INES_MAGIC3 'S'
#define INES_MAGIC4 0x1a

#define INES_PRG_ROM_MULTIPLIER 16
#define INES_PRG_RAM_MULTIPLIER 8
#define INES_CHR_ROM_MULTIPLIER 8

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct iNesHeader {
    /* 'NES^A' */
    uint8_t magic[4];
    /* Rom size */
    uint8_t prg_rom_size;
    /* VRom size */
    uint8_t chr_rom_size;
    /* Flag6 */
    uint8_t flag6_mirroring         : 1;
    uint8_t flag6_persitant         : 1;
    uint8_t flag6_trainer           : 1;
    uint8_t flag6_4screen_vram      : 1;
    uint8_t flag6_low_mapper_nibble : 4;
    /* Flag7 */
    uint8_t flag7_vs_inusystem        : 1;
    uint8_t flag7_playchoice10        : 1;
    uint8_t flag7_ines_version        : 2;
    uint8_t flag7_upper_mapper_nibble : 4;
    /* Flag8 */
    uint8_t flag8_prg_ram_size;
    /* Flag9 */
    uint8_t flag9_tv_system : 1;
    uint8_t flag9_reserved : 7;
    /* Padding */
    uint8_t padding[6];
};

struct iNes {
    struct iNesHeader header;
    uint8_t *trainer;
    uint8_t *prg_rom;
    uint8_t *chr_rom;
    uint8_t *inst_rom;
    uint8_t *prom;
};

#define INES_MAX_SECTION 4

struct mapper_support {
    char support;
    char mapper_id;
    uint8_t nb_sections;
    struct ines_mapper_section sections[INES_MAX_SECTION];
};

const struct mapper_support mappers[] = {
    {
        .support = 0,
        .mapper_id = 0x0,
        .nb_sections = 2,
        .sections = {
            {
                .section_id = INES_SECTION_PRG_RAM,
                .start = 0x6000,
                .end = 0x7FFF
            },
            {
                .section_id = INES_SECTION_PRG_ROM,
                .start = 0x8000,
                .end = 0xFFFF
            }
        }
    }
};

static int ines_fd = -1;
static struct iNes loaded = { 0 };

static int extract_prg_rom(void)
{
    /* ines_fd must be open and header already read */
    ssize_t s = loaded.header.prg_rom_size * INES_CHR_ROM_MULTIPLIER * 1024;
    loaded.prg_rom = malloc(s);
    s = read(ines_fd, loaded.prg_rom, s);
    if (s < 0) {
        free(loaded.prg_rom);
        loaded.prg_rom = 0;
        return -1;
    }
    return 0;
}

static int extract_chr_rom(void)
{
    /* ines_fd must be open and header, chr_rom already read */
    ssize_t s = loaded.header.chr_rom_size * INES_CHR_ROM_MULTIPLIER * 1024;
    loaded.chr_rom = malloc(s);
    s = read(ines_fd, loaded.chr_rom, s);
    if (s < 0) {
        free(loaded.chr_rom);
        loaded.chr_rom = 0;
        return -1;
    }
    return 0;
}

static int get_mapper_idx(uint8_t id)
{
    int i = 0;
    while (i < ARRAY_SIZE(mappers) && mappers[i].mapper_id != id)
        ++i;
    if (i == ARRAY_SIZE(mappers))
        return -ENOENT;

    return i;
}

int ines_load_file(const char *path)
{
    ssize_t size_ret;
    int ret = 0;

    ines_fd = open(path, O_RDONLY);
    if (ines_fd < 0) {
        return ines_fd;
    }

    size_ret = read(ines_fd, &loaded, sizeof(loaded.header));
    if (size_ret < 0) {
        return size_ret;
    }

#if defined DEBUG
    debug("PRG_ROM size = %d Kio\n", loaded.header.prg_rom_size * INES_PRG_ROM_MULTIPLIER);
    debug("CHR_ROM size = %d Kio\n", loaded.header.chr_rom_size * INES_CHR_ROM_MULTIPLIER);
    debug("Flag 6:\n");
    debug("\tMirroring = %s\n", loaded.header.flag6_mirroring ? INES_FLAG6_MIRRORING_V_NAME :
            INES_FLAG6_MIRRORING_H_NAME);
    debug("\tPersistant = %d\n", loaded.header.flag6_persitant);
    debug("\tTrainer = %d\n", loaded.header.flag6_trainer);
    debug("\t4 Screen VRAM = %d\n", loaded.header.flag6_4screen_vram);
    debug("\tLow mapper nibble = %x\n", loaded.header.flag6_low_mapper_nibble);
    debug("Flag 7:\n");
    debug("\tVS Unisystem = %d\n", loaded.header.flag7_vs_inusystem);
    debug("\tPlaychoice 10 = %d\n", loaded.header.flag7_playchoice10);
    debug("\tiNes version = %x\n", loaded.header.flag7_ines_version);
    debug("\tUpper mapper nibble = %x\n", loaded.header.flag7_upper_mapper_nibble);
    debug("Flag 8: PRG_RAM size = %d Kio\n", loaded.header.flag8_prg_ram_size * INES_PRG_RAM_MULTIPLIER);
    debug("Flag 9: tv system = %d\n", loaded.header.flag9_tv_system);
#endif // DEBUG

    ret = ines_is_supported();
    if (ret < 0) {
        goto reset;
    }

    ret = extract_prg_rom();
    if (ret < 0) {
        goto reset;
    }

    ret = extract_chr_rom();
    if (ret < 0) {
        goto reset;
    }

    return 0;

reset:
    ines_close_file();
    return ret;
}

void ines_close_file(void)
{
    close(ines_fd);

    if (loaded.prg_rom != 0) {
        free(loaded.prg_rom);
        loaded.prg_rom = 0;
    }

    if (loaded.chr_rom != 0) {
        free(loaded.chr_rom);
        loaded.chr_rom = 0;
    }
}

int ines_is_supported(void)
{
    int ret = 0;
    char mapper_id = loaded.header.flag7_upper_mapper_nibble << 4
        | loaded.header.flag6_low_mapper_nibble;

    /* Check magic */
    if (loaded.header.magic[0] != INES_MAGIC1
            || loaded.header.magic[1] != INES_MAGIC2
            || loaded.header.magic[2] != INES_MAGIC3
            || loaded.header.magic[3] != INES_MAGIC4)
        return -EINVAL;

    /* Check if mapper is supported */
    ret = get_mapper_idx(mapper_id);
    if (ret < 0) {
        return ret;
    }

    return 0;
}

int ines_get_sections(const struct ines_mapper_section **sections)
{
    int mapper_idx;
    char mapper_id = loaded.header.flag7_upper_mapper_nibble << 4
        | loaded.header.flag6_low_mapper_nibble;

    mapper_idx = get_mapper_idx(mapper_id);
    *sections = mappers[mapper_idx].sections;

    return mappers[mapper_idx].nb_sections;
}

int ines_get_section_by_id(uint8_t id, void **rom, size_t *size)
{
    int ret = 0;

    ret = ines_is_supported();
    if (ret < 0) {
        return ret;
    }

    if (id != INES_SECTION_PRG_ROM)
        return -ENOENT;

    *rom = loaded.prg_rom;
    *size = loaded.header.prg_rom_size;

    return 0;
}

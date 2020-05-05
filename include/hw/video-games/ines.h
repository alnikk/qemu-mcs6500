#ifndef INES_H
#define INES_H

#define INES_SECTION_PRG_RAM 0
#define INES_SECTION_PRG_ROM 1

struct ines_mapper_section {
    uint8_t section_id;
    uint16_t start;
    uint16_t end;
};

int ines_load_file(const char *path);
int ines_is_supported(void);
void ines_close_file(void);

int ines_get_sections(const struct ines_mapper_section **sections);

int ines_get_section_by_id(uint8_t id, void **rom, size_t *size);

#endif // INES_H

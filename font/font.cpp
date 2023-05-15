#include "font.h"

Font::Font(const Ctab_entry* const ctab_, const uint8_t* const cdat_, const uint8_t cht_) :
    ctab(ctab_), cdat(cdat_), cht(cht_) {}

uint8_t Font::get_length(char c) const {
    if (c < 0x20 || c >= 0x7F) return 0;
    return ctab[c-0x20].length;
}

const uint8_t* Font::get_data(char c) const {
    if (c < 0x20 || c >= 0x7F) return nullptr;
    const Ctab_entry* const entry = ctab + (c-0x20);
    if (entry->length == 0) return nullptr;
    unsigned int offset = entry->offset;
    return cdat + offset;
};

const uint16_t* Font::get_tall_data(char c) const {
    if (c < 0x20 || c >= 0x7F) return nullptr;
    const Ctab_entry* const entry = ctab + (c-0x20);
    if (entry->length == 0) return nullptr;
    unsigned int offset = entry->offset;
    return ((uint16_t*)cdat) + offset;
};

uint16_t Font::width(const char* c) const {
    uint16_t w = 0;
    while (*c != 0) {
        w += get_length(*(c++));
        w += 1; // intercharacter space
    }
    return w;
}

uint8_t Font::height() const {
    return cht;
}

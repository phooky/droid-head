#include "font.h"

const CharInfo get_char(const FontInfo* f, char c) {
    if (c < 0x20 || c >= 0x7F) { return (const CharInfo){0,0}; }
    const Ctab_entry te = f->ctab[c-0x20];
    return (const CharInfo){te.length, f->cdata + te.offset};
}

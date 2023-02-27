#include "font.h"

CharInfo get_char(FontInfo* f, char c) {
    if (c < 0x20 || c >= 0x7F) { return (CharInfo){0,0}; }
    Ctab_entry te = f->ctab[c-0x20];
    return (CharInfo){te.length, f->cdata + te.offset};
}

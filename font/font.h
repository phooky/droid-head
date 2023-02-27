#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct {
    uint8_t length;
    uint8_t* data;
} CharInfo;

typedef struct __attribute__((packed)) {
    unsigned int length : 4;
    unsigned int offset : 12;
} Ctab_entry;

typedef struct {
    Ctab_entry* ctab;
    uint8_t* cdata;
} FontInfo;

CharInfo get_char(FontInfo* f, char c);

#endif // __FONT_H__

#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct __attribute__((packed)) {
    unsigned int length : 4;
    unsigned int offset : 12;
} Ctab_entry;

class Font {
 private:
    const Ctab_entry* const ctab;
    const uint8_t* const cdat;
    const uint8_t cht;
 public:
    Font(const Ctab_entry* const ctab, const uint8_t* const cdat, const uint8_t cht = 8);
    uint8_t get_length(char c) const;
    const uint8_t* get_data(char c) const;
    const uint16_t* get_tall_data(char c) const;

    uint16_t width(const char* c) const;
    uint8_t height() const;

    bool tall() const { return height() > 8; }
};

#endif // __FONT_H__

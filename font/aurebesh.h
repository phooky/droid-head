
#ifndef __AUREBESH_H__
#define __AUREBESH_H__

#include "font.h"

extern const Ctab_entry* const aurebesh_ctab;
extern const uint8_t* const aurebesh_cdata;

const Font aurebesh_f(aurebesh_ctab, aurebesh_cdata);

#endif // __AUREBESH_H__

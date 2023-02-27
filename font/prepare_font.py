#!/usr/bin/env python

import os
import sys
from PIL import Image

fontconfig = sys.argv[1]
base_dir = os.path.split(fontconfig)[0]

params = {}
for l in open(fontconfig).readlines():
    (k,v) = l.split(maxsplit=1)
    params[k]=v.strip()
    
img_path = params['source']
characters = params['characters']
name = params['name']

img = Image.open(img_path).convert("L")
print (img.format, img.size, img.mode)

def getCol(col):
    v = 0
    for i in range(8):
        p = img.getpixel((col,i))
        v = v << 1
        if p == 0:
            v = v | 1
    return v


left = 0
font = {}
for c in characters:
    d = [];
    while True:
        l = getCol(left)
        left = left + 1
        if l == 0:
            break
        d.append(l)
    font[c] = d

hpath = os.path.join(base_dir,name+'.h')
cpath = os.path.join(base_dir,name+'.c')

h=open(hpath,'w')
h.write(f"""
#ifndef __{name.upper()}_H__
#define __{name.upper()}_H__

#include "font.h"
Ctab_entry {name}_ctab[];
uint8_t {name}_cdata[];

FontInfo {name}_fi = {{ (Ctab_entry*){name}_ctab, (uint8_t*){name}_cdata }};

#endif // __{name.upper()}_H__
""");
h.close()

# Prepare ctab/cdata
ctab = ""
cdata = []

for c in range(0x20,0x7f):
    try:
        dat = font[chr(c)]
        ctab += f"  {{{len(dat)},{len(cdata)}}},\n"
        cdata += dat
    except:
        ctab += "  {0,0},\n"

cdata = ", ".join(map(hex,cdata))

c=open(cpath,'w')
c.write(f"""
#include "{name}.h"

Ctab_entry {name}_ctab[] = {{
{ctab}
}};

uint8_t {name}_cdata[] = {{
{cdata}
}};

""");
c.close()


#pragma once

#include <common.h>
#include <spm/filemgr.h>
#include <wii/gx.h>
#include <stdint.h>

CPP_WRAPPER(spm::eff_sub)

USING(spm::filemgr::FileEntry)

typedef struct
{
/* 0x000 */ FileEntry * activeTplFile;
/* 0x004 */ FileEntry * activeDatFile;
/* 0x008 */ void * datBody;
/* 0x00C */ FileEntry * effdataTplFile;
/* 0x010 */ FileEntry * effdataDatFile;
/* 0x014 */ uint32_t effdataDatUnk;

/* 0x018 */ char* loadedEffectName;
/* 0x01C */ char* effectNameToLoad;
/* 0x020 */ FileEntry* effectTplFp;
/* 0x024 */ FileEntry* effectDatFp;
/* 0x028 */ uint32_t effectDatUnk;

    uint8_t unk_2C[0x78 - 0x2C];

/* 0x78 */ float dispBoundaryOriginX;
/* 0x7C */ float dispBoundaryOriginZ;
/* 0x80 */ float dispBoundaryWidthX;
/* 0x84 */ float dispBoundaryWidthZ;

    uint8_t unk_88[0xA4 - 0x88];

/* 0xA4 */ int frameCount;

    uint8_t unk_A8[0xD4 - 0xA8];

/* 0xD4 */ void* texBufferSp;

    uint8_t unk_D8[0xE0 - 0xD8];

/* 0xE0 */ uint8_t overrideEffColor;

    uint8_t unk_E1[0xEC - 0xE1];

/* 0xEC */ uint32_t specularUnk;
/* 0xF0 */ uint32_t indtextureUnk;
/* 0xF4 */ uint32_t indTexture1Unk;
/* 0xF8 */ uint32_t indTexture2Unk;
/* 0xFC */ uint32_t clipTextureUnk;
} EffSubWork;
SIZE_ASSERT(EffSubWork, 0x100)

DECOMP_STATIC(EffSubWork * effsub_wp)

void effSubMain();

CPP_WRAPPER_END()

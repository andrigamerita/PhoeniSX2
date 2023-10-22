/*  AetherSX2 - PS2 Emulator for Android and ARM PCs
 *  Copyright (C) 2022 AetherSX2 Dev Team
 *
 *  AetherSX2 is provided under the terms of the Creative Commons
 *  Attribution-NonCommercial-NoDerivatives International License
 *  (BY-NC-ND 4.0, https://creativecommons.org/licenses/by-nc-nd/4.0/).
 * 
 *  Commercialization of this application and source code is forbidden.
 */

#pragma once

#include "Vif.h"
#include "VU.h"

#include "System/RecTypes.h"

// newVif_HashBucket.h uses this typedef, so it has to be declared first.
typedef u32  (__fastcall *nVifCall)(void*, const void*);
typedef void (__fastcall *nVifrecCall)(uptr dest, uptr src);

#include "newVif_HashBucket.h"

extern void _nVifUnpack  (int idx, const u8* data, uint mode, bool isFill);
extern void  dVifReserve (int idx);
extern void  dVifReset   (int idx);
extern void  dVifClose   (int idx);
extern void  dVifRelease (int idx);
extern void  VifUnpackSSE_Init();
extern void  VifUnpackSSE_Destroy();

_vifT extern void  dVifUnpack  (const u8* data, bool isFill);

#define VUFT VIFUnpackFuncTable
#define xmmCol0 vixl::aarch64::q2
#define xmmCol1 vixl::aarch64::q3
#define xmmCol2 vixl::aarch64::q4
#define xmmCol3 vixl::aarch64::q5
#define xmmRow  vixl::aarch64::q6
#define xmmTemp vixl::aarch64::q7

struct nVifStruct
{
	// Buffer for partial transfers (should always be first to ensure alignment)
	// Maximum buffer size is 256 (vifRegs.Num max range) * 16 (quadword)
	alignas(16) u8 buffer[256 * 16];
	u32            bSize; // Size of 'buffer'

	// VIF0 or VIF1 - provided for debugging helpfulness only, and is generally unused.
	// (templates are used for most or all VIF indexing)
	u32                     idx;

	RecompiledCodeReserve* recReserve;
	u8* recWritePtr; // current write pos into the reserve

	HashBucket              vifBlocks;   // Vif Blocks


	nVifStruct() = default;
};

extern void resetNewVif(int idx);
extern void releaseNewVif(int idx);

alignas(16) extern nVifStruct nVif[2];
alignas(16) extern nVifCall nVifUpk[(2 * 2 * 16) * 4]; // ([USN][Masking][Unpack Type]) [curCycle]
alignas(16) extern u32      nVifMask[3][4][4];         // [MaskNumber][CycleNumber][Vector]

static const bool newVifDynaRec = 1; // Use code in newVif_Dynarec.inl

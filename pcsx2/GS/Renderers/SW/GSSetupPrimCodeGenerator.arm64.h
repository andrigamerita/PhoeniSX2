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

#include "GSScanlineEnvironment.h"

#include "vixl/aarch64/macro-assembler-aarch64.h"

class GSSetupPrimCodeGenerator2
{
public:
	GSSetupPrimCodeGenerator2(vixl::aarch64::MacroAssembler& armAsm_, void* param, u64 key);
	void Generate();

private:
	void Depth_NEON();
	void Texture_NEON();
	void Color_NEON();

	vixl::aarch64::MacroAssembler& armAsm;

	GSScanlineSelector m_sel;
	GSScanlineLocalData& m_local;
	bool m_rip;
	bool many_regs;

	struct
	{
		u32 z : 1, f : 1, t : 1, c : 1;
	} m_en;
};

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

#include "PCSX2Base.h"
#include <string>
#include <vector>

namespace GSDumpReplayer
{
bool IsReplayingDump();

bool Initialize(const char* filename);
void Reset();
void Shutdown();

std::string GetDumpSerial();
u32 GetDumpCRC();

void RenderUI();
}
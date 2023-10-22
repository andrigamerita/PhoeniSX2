/*  AetherSX2 - PS2 Emulator for Android and ARM PCs
 *  Copyright (C) 2022 AetherSX2 Dev Team
 *
 *  AetherSX2 is provided under the terms of the Creative Commons
 *  Attribution-NonCommercial-NoDerivatives International License
 *  (BY-NC-ND 4.0, https://creativecommons.org/licenses/by-nc-nd/4.0/).
 * 
 *  Commercialization of this application and source code is forbidden.
 */

#include "PrecompiledHeader.h"
#include "Host.h"
#include "common/StringHelpers.h"
#include <cstdarg>

void Host::ReportFormattedErrorAsync(const std::string_view& title, const char* format, ...)
{
	std::va_list ap;
	va_start(ap, format);
	FastFormatAscii fmt;
	fmt.WriteV(format, ap);
	va_end(ap);
	ReportErrorAsync(title, fmt.c_str());
}
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

#include "common/Pcsx2Defs.h"

#include <string>
#include <string_view>
#include <optional>
#include <vector>

struct HostKeyEvent
{
	enum class Type
	{
		NoEvent,
		KeyPressed,
		KeyReleased,
		MousePressed,
		MouseReleased,
		MouseWheelDown,
		MouseWheelUp,
		MouseMove,
		FocusGained,
		FocustLost,
	};

	Type type;
	u32 key;
};

namespace Host
{
	/// Reads a file from the resources directory of the application.
	/// This may be outside of the "normal" filesystem on platforms such as Mac.
	std::optional<std::vector<u8>> ReadResourceFile(const char* filename);

	/// Reads a resource file file from the resources directory as a string.
	std::optional<std::string> ReadResourceFileToString(const char* filename);

	/// Adds OSD messages, duration is in seconds.
	void AddOSDMessage(std::string message, float duration = 2.0f);
	void AddKeyedOSDMessage(std::string key, std::string message, float duration = 2.0f);
	void AddFormattedOSDMessage(float duration, const char* format, ...);
	void AddKeyedFormattedOSDMessage(std::string key, float duration, const char* format, ...);
	void RemoveKeyedOSDMessage(std::string key);
	void ClearOSDMessages();

	/// Displays an asynchronous error on the UI thread, i.e. doesn't block the caller.
	void ReportErrorAsync(const std::string_view& title, const std::string_view& message);
	void ReportFormattedErrorAsync(const std::string_view& title, const char* format, ...);
} // namespace Host

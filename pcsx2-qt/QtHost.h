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
#include "pcsx2/Host.h"
#include "pcsx2/HostDisplay.h"
#include "pcsx2/HostSettings.h"
#include "pcsx2/Frontend/InputManager.h"
#include "pcsx2/VMManager.h"
#include <QtCore/QMetaType>
#include <functional>
#include <memory>
#include <optional>

class SettingsInterface;

class EmuThread;

Q_DECLARE_METATYPE(std::shared_ptr<VMBootParameters>);
Q_DECLARE_METATYPE(std::optional<bool>);
Q_DECLARE_METATYPE(std::function<void()>);
Q_DECLARE_METATYPE(GSRendererType);
Q_DECLARE_METATYPE(InputBindingKey);

namespace QtHost
{
	bool Initialize();
	void Shutdown();

	void UpdateFolders();
	void UpdateLogging();

	/// Initializes early console logging (for printing command line arguments).
	void InitializeEarlyConsole();

	/// Sets batch mode (exit after game shutdown).
	bool InBatchMode();
	void SetBatchMode(bool enabled);

	/// Executes a function on the UI thread.
	void RunOnUIThread(const std::function<void()>& func, bool block = false);

	/// Thread-safe settings access.
	SettingsInterface* GetBaseSettingsInterface();
	std::string GetBaseStringSettingValue(const char* section, const char* key, const char* default_value = "");
	bool GetBaseBoolSettingValue(const char* section, const char* key, bool default_value = false);
	int GetBaseIntSettingValue(const char* section, const char* key, int default_value = 0);
	float GetBaseFloatSettingValue(const char* section, const char* key, float default_value = 0.0f);
	std::vector<std::string> GetBaseStringListSetting(const char* section, const char* key);
	void SetBaseBoolSettingValue(const char* section, const char* key, bool value);
	void SetBaseIntSettingValue(const char* section, const char* key, int value);
	void SetBaseFloatSettingValue(const char* section, const char* key, float value);
	void SetBaseStringSettingValue(const char* section, const char* key, const char* value);
	void SetBaseStringListSettingValue(const char* section, const char* key, const std::vector<std::string>& values);
	bool AddBaseValueToStringList(const char* section, const char* key, const char* value);
	bool RemoveBaseValueFromStringList(const char* section, const char* key, const char* value);
	void RemoveBaseSettingValue(const char* section, const char* key);
	void QueueSettingsSave();
} // namespace QtHost

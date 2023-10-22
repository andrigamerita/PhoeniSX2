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
#include "GameDatabase.h"
#include "common/Pcsx2Defs.h"
#include <ctime>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class ProgressCallback;

struct VMBootParameters;

namespace GameList
{
	enum class EntryType
	{
		PS2Disc,
		PS1Disc,
		ELF,
		Playlist,
		Count
	};

	enum class Region
	{
		NTSC_UC,
		NTSC_J,
		PAL,
		Other,
		Count
	};

	using CompatibilityRating = GameDatabaseSchema::Compatibility;
	static constexpr u32 CompatibilityRatingCount = static_cast<u32>(GameDatabaseSchema::Compatibility::Perfect) + 1u;

	struct Entry
	{
		EntryType type = EntryType::PS2Disc;
		Region region = Region::Other;

		std::string path;
		std::string serial;
		std::string title;
		u64 total_size = 0;
		std::time_t last_modified_time = 0;

		u32 crc = 0;

		CompatibilityRating compatibility_rating = CompatibilityRating::Unknown;
	};

	const char* EntryTypeToString(EntryType type);
	const char* EntryCompatibilityRatingToString(CompatibilityRating rating);

	bool IsScannableFilename(const std::string& path);

	/// Fills in boot parameters (iso or elf) based on the game list entry.
	void FillBootParametersForEntry(VMBootParameters* params, const Entry* entry);

	/// Populates a game list entry struct with information from the iso/elf.
	/// Do *not* call while the system is running, it will mess with CDVD state.
	bool PopulateEntryFromPath(const std::string& path, GameList::Entry* entry);

	// Game list access. It's the caller's responsibility to hold the lock while manipulating the entry in any way.
	std::unique_lock<std::recursive_mutex> GetLock();
	const Entry* GetEntryByIndex(u32 index);
	const Entry* GetEntryForPath(const char* path);
	const Entry* GetEntryByCRC(u32 crc);
	const Entry* GetEntryBySerialAndCRC(const std::string_view& serial, u32 crc);
	u32 GetEntryCount();

	bool IsGameListLoaded();
	void Refresh(bool invalidate_cache, ProgressCallback* progress = nullptr);

	std::string GetCoverImagePathForEntry(const Entry* entry);
	std::string GetCoverImagePath(const std::string& path, const std::string& code, const std::string& title);
	std::string GetNewCoverImagePathForEntry(const Entry* entry, const char* new_filename);
} // namespace GameList

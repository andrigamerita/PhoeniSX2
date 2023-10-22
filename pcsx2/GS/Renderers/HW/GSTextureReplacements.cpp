/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2022  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"

#include "common/HashCombine.h"
#include "common/FileSystem.h"
#include "common/Path.h"
#include "common/StringUtil.h"
#include "common/ScopedGuard.h"

#include "Config.h"
#include "GS/GSLocalMemory.h"
#include "GS/Renderers/HW/GSTextureReplacements.h"

#ifndef PCSX2_CORE
#include "gui/AppCoreThread.h"
#else
#include "VMManager.h"
#endif

#include "bc7decomp.h"
#include "rgbcx.h"

#include <cinttypes>
#include <cstring>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <tuple>
#include <thread>

// this is a #define instead of a variable to avoid warnings from non-literal format strings
#define TEXTURE_FILENAME_FORMAT_STRING "%" PRIx64 "-%08x"
#define TEXTURE_FILENAME_CLUT_FORMAT_STRING "%" PRIx64 "-%" PRIx64 "-%08x"
#define TEXTURE_REPLACEMENT_SUBDIRECTORY_NAME "replacements"
#define TEXTURE_DUMP_SUBDIRECTORY_NAME "dumps"

namespace
{
	struct TextureName // 24 bytes
	{
		u64 TEX0Hash;
		u64 CLUTHash;

		union
		{
			struct
			{
				u32 TEX0_PSM : 6;
				u32 TEX0_TW : 4;
				u32 TEX0_TH : 4;
				u32 TEX0_TCC : 1;
				u32 TEXA_TA0 : 8;
				u32 TEXA_AEM : 1;
				u32 TEXA_TA1 : 8;
			};
			u32 bits;
		};
		u32 miplevel;

		__fi u32 Width() const { return (1u << TEX0_TW); }
		__fi u32 Height() const { return (1u << TEX0_TH); }
		__fi bool HasPalette() const { return (GSLocalMemory::m_psm[TEX0_PSM].pal > 0); }

		__fi GSVector2 ReplacementScale(const GSTextureReplacements::ReplacementTexture& rtex) const
		{
			return ReplacementScale(rtex.width, rtex.height);
		}

		__fi GSVector2 ReplacementScale(u32 rwidth, u32 rheight) const
		{
			return GSVector2(static_cast<float>(rwidth) / static_cast<float>(Width()), static_cast<float>(rheight) / static_cast<float>(Height()));
		}

		__fi bool operator==(const TextureName& rhs) const { return std::tie(TEX0Hash, CLUTHash, bits) == std::tie(rhs.TEX0Hash, rhs.CLUTHash, rhs.bits); }
		__fi bool operator!=(const TextureName& rhs) const { return std::tie(TEX0Hash, CLUTHash, bits) != std::tie(rhs.TEX0Hash, rhs.CLUTHash, rhs.bits); }
		__fi bool operator<(const TextureName& rhs) const { return std::tie(TEX0Hash, CLUTHash, bits) < std::tie(rhs.TEX0Hash, rhs.CLUTHash, rhs.bits); }
	};
	static_assert(sizeof(TextureName) == 24, "ReplacementTextureName is expected size");
} // namespace

namespace std
{
	template <>
	struct hash<TextureName>
	{
		std::size_t operator()(const TextureName& val) const
		{
			std::size_t h = 0;
			HashCombine(h, val.TEX0Hash, val.CLUTHash, val.bits, val.miplevel);
			return h;
		}
	};
} // namespace std

namespace GSTextureReplacements
{
	static TextureName CreateTextureName(const GSTextureCache::HashCacheKey& hash, u32 miplevel);
	static GSTextureCache::HashCacheKey HashCacheKeyFromTextureName(const TextureName& tn);
	static std::optional<TextureName> ParseReplacementName(const std::string& filename);
	static std::string GetGameTextureDirectory();
	static std::string GetDumpFilename(const TextureName& name, u32 level);
	static std::string GetGameSerial();
	static bool IsFormatSupported(GSTexture::Format format);
	static std::optional<ReplacementTexture> LoadReplacementTexture(const TextureName& name, const std::string& filename, bool only_base_image);
	static void DecompressReplacementTexture(ReplacementTexture& rtex);
	static void QueueAsyncReplacementTextureLoad(const TextureName& name, const std::string& filename, bool mipmap);
	static void PrecacheReplacementTextures();
	static void ClearReplacementTextures();

	static void StartWorkerThread();
	static void StopWorkerThread();
	static void QueueWorkerThreadItem(std::function<void()> fn);
	static void WorkerThreadEntryPoint();
	static void SyncWorkerThread();
	static void CancelPendingLoadsAndDumps();

	static std::string s_current_serial;

	/// Backreference to the texture cache so we can inject replacements.
	static GSTextureCache* s_tc;

	/// Textures that have been dumped, to save stat() calls.
	static std::unordered_set<TextureName> s_dumped_textures;

	/// Lookup map of texture names to replacements, if they exist.
	static std::unordered_map<TextureName, std::string> s_replacement_texture_filenames;

	/// Lookup map of texture names to replacement data which has been cached.
	static std::unordered_map<TextureName, ReplacementTexture> s_replacement_texture_cache;
	static std::mutex s_replacement_texture_cache_mutex;

	/// List of textures that are pending asynchronous load.
	static std::unordered_set<TextureName> s_pending_async_load_textures;

	/// List of textures that we have asynchronously loaded and can now be injected back into the TC.
	/// Second element is whether the texture should be created with mipmaps.
	static std::vector<std::pair<TextureName, bool>> s_async_loaded_textures;

	/// Loader/dumper thread.
	static std::thread s_worker_thread;
	static std::mutex s_worker_thread_mutex;
	static std::condition_variable s_worker_thread_cv;
	static std::queue<std::function<void()>> s_worker_thread_queue;
	static bool s_worker_thread_running = false;
}; // namespace GSTextureReplacements

TextureName GSTextureReplacements::CreateTextureName(const GSTextureCache::HashCacheKey& hash, u32 miplevel)
{
	TextureName name;
	name.bits = 0;
	name.TEX0_PSM = hash.TEX0.PSM;
	name.TEX0_TW = hash.TEX0.TW;
	name.TEX0_TH = hash.TEX0.TH;
	name.TEX0_TCC = hash.TEX0.TCC;
	name.TEXA_TA0 = hash.TEXA.TA0;
	name.TEXA_AEM = hash.TEXA.AEM;
	name.TEXA_TA1 = hash.TEXA.TA1;
	name.TEX0Hash = hash.TEX0Hash;
	name.CLUTHash = name.HasPalette() ? hash.CLUTHash : 0;
	name.miplevel = miplevel;
	return name;
}

GSTextureCache::HashCacheKey GSTextureReplacements::HashCacheKeyFromTextureName(const TextureName& tn)
{
	GSTextureCache::HashCacheKey key = {};
	key.TEX0.PSM = tn.TEX0_PSM;
	key.TEX0.TW = tn.TEX0_TW;
	key.TEX0.TH = tn.TEX0_TH;
	key.TEX0.TCC = tn.TEX0_TCC;
	key.TEXA.TA0 = tn.TEXA_TA0;
	key.TEXA.AEM = tn.TEXA_AEM;
	key.TEXA.TA1 = tn.TEXA_TA1;
	key.TEX0Hash = tn.TEX0Hash;
	key.CLUTHash = tn.HasPalette() ? tn.CLUTHash : 0;
	return key;
}

std::optional<TextureName> GSTextureReplacements::ParseReplacementName(const std::string& filename)
{
	TextureName ret;
	ret.miplevel = 0;

	// TODO(Stenzek): Make this better.
	char extension_dot;
	if (std::sscanf(filename.c_str(), TEXTURE_FILENAME_CLUT_FORMAT_STRING "%c", &ret.TEX0Hash, &ret.CLUTHash, &ret.bits, &extension_dot) != 4 || extension_dot != '.')
	{
		if (std::sscanf(filename.c_str(), TEXTURE_FILENAME_FORMAT_STRING "%c", &ret.TEX0Hash, &ret.bits, &extension_dot) != 3 || extension_dot != '.')
			return std::nullopt;
	}

	return ret;
}

std::string GSTextureReplacements::GetGameTextureDirectory()
{
	return Path::CombineStdString(EmuFolders::Textures, s_current_serial);
}

std::string GSTextureReplacements::GetDumpFilename(const TextureName& name, u32 level)
{
	std::string ret;
	if (s_current_serial.empty())
		return ret;

	const std::string game_dir(GetGameTextureDirectory());
	if (!FileSystem::DirectoryExists(game_dir.c_str()))
	{
		// create both dumps and replacements
		if (!FileSystem::CreateDirectoryPath(game_dir.c_str(), false) ||
			!FileSystem::EnsureDirectoryExists(Path::CombineStdString(game_dir, "dumps").c_str(), false) ||
			!FileSystem::EnsureDirectoryExists(Path::CombineStdString(game_dir, "replacements").c_str(), false))
		{
			// if it fails to create, we're not going to be able to use it anyway
			return ret;
		}
	}

	const std::string game_subdir(Path::CombineStdString(game_dir, TEXTURE_DUMP_SUBDIRECTORY_NAME));

	if (name.HasPalette())
	{
		const std::string filename(
			(level > 0) ?
                StringUtil::StdStringFromFormat(TEXTURE_FILENAME_CLUT_FORMAT_STRING "-mip%u.png", name.TEX0Hash, name.CLUTHash, name.bits, level) :
                StringUtil::StdStringFromFormat(TEXTURE_FILENAME_CLUT_FORMAT_STRING ".png", name.TEX0Hash, name.CLUTHash, name.bits));
		ret = Path::CombineStdString(game_subdir, filename);
	}
	else
	{
		const std::string filename(
			(level > 0) ?
                StringUtil::StdStringFromFormat(TEXTURE_FILENAME_FORMAT_STRING "-mip%u.png", name.TEX0Hash, name.bits, level) :
                StringUtil::StdStringFromFormat(TEXTURE_FILENAME_FORMAT_STRING ".png", name.TEX0Hash, name.bits));
		ret = Path::CombineStdString(game_subdir, filename);
	}

	return ret;
}

std::string GSTextureReplacements::GetGameSerial()
{
#ifndef PCSX2_CORE
	return StringUtil::wxStringToUTF8String(GameInfo::gameSerial);
#else
	return VMManager::GetGameSerial();
#endif
}

bool GSTextureReplacements::IsFormatSupported(GSTexture::Format format)
{
	switch (format)
	{
	case GSTexture::Format::Color:
		return true;

	case GSTexture::Format::BC1:
	case GSTexture::Format::BC2:
	case GSTexture::Format::BC3:
		return g_gs_device->Features().dxt_textures;

	case GSTexture::Format::BC7:
		return g_gs_device->Features().bptc_textures;

	default:
		return false;
	}
}

void GSTextureReplacements::Initialize(GSTextureCache* tc)
{
	s_tc = tc;
	s_current_serial = GetGameSerial();

	if (GSConfig.DumpReplaceableTextures || GSConfig.LoadTextureReplacements)
		StartWorkerThread();

	ReloadReplacementMap();
}

void GSTextureReplacements::GameChanged()
{
	if (!s_tc)
		return;

	std::string new_serial(GetGameSerial());
	if (s_current_serial == new_serial)
		return;

	s_current_serial = std::move(new_serial);
	ReloadReplacementMap();
	ClearDumpedTextureList();
}

void GSTextureReplacements::ReloadReplacementMap()
{
	SyncWorkerThread();

	// clear out the caches
	{
		std::unique_lock<std::mutex> lock(s_replacement_texture_cache_mutex);
		s_replacement_texture_cache.clear();
		s_replacement_texture_filenames.clear();
		s_pending_async_load_textures.clear();
		s_async_loaded_textures.clear();
	}

	// can't replace bios textures.
	if (s_current_serial.empty() || !GSConfig.LoadTextureReplacements)
		return;

	const std::string replacement_dir(Path::CombineStdString(GetGameTextureDirectory(), TEXTURE_REPLACEMENT_SUBDIRECTORY_NAME));

	FileSystem::FindResultsArray files;
	if (!FileSystem::FindFiles(replacement_dir.c_str(), "*", FILESYSTEM_FIND_FILES | FILESYSTEM_FIND_HIDDEN_FILES | FILESYSTEM_FIND_RECURSIVE, &files))
		return;

	std::string filename;
	for (FILESYSTEM_FIND_DATA& fd : files)
	{
		// file format we can handle?
		filename = FileSystem::GetFileNameFromPath(fd.FileName);
		if (!GetLoader(filename))
			continue;

		// parse the name if it's valid
		std::optional<TextureName> name = ParseReplacementName(filename);
		if (!name.has_value())
			continue;

		DevCon.WriteLn("Found %ux%u replacement '%.*s'", name->Width(), name->Height(), static_cast<int>(filename.size()), filename.data());
		s_replacement_texture_filenames.emplace(std::move(name.value()), std::move(fd.FileName));
	}

	if (GSConfig.PrecacheTextureReplacements)
		PrecacheReplacementTextures();
}

void GSTextureReplacements::UpdateConfig(Pcsx2Config::GSOptions& old_config)
{
	// get rid of worker thread if it's no longer needed
	if (s_worker_thread_running && !GSConfig.DumpReplaceableTextures && !GSConfig.LoadTextureReplacements)
		StopWorkerThread();
	if (!s_worker_thread_running && (GSConfig.DumpReplaceableTextures || GSConfig.LoadTextureReplacements))
		StartWorkerThread();

	if ((!GSConfig.DumpReplaceableTextures && old_config.DumpReplaceableTextures) ||
		(!GSConfig.LoadTextureReplacements && old_config.LoadTextureReplacements))
	{
		CancelPendingLoadsAndDumps();
	}

	if (GSConfig.LoadTextureReplacements && !old_config.LoadTextureReplacements)
		ReloadReplacementMap();
	else if (!GSConfig.LoadTextureReplacements && old_config.LoadTextureReplacements)
		ClearReplacementTextures();

	if (!GSConfig.DumpReplaceableTextures && old_config.DumpReplaceableTextures)
		ClearDumpedTextureList();

	if (GSConfig.LoadTextureReplacements && GSConfig.PrecacheTextureReplacements && !old_config.PrecacheTextureReplacements)
		PrecacheReplacementTextures();
}

void GSTextureReplacements::Shutdown()
{
	StopWorkerThread();

	std::string().swap(s_current_serial);
	ClearReplacementTextures();
	ClearDumpedTextureList();
	s_tc = nullptr;
}

u32 GSTextureReplacements::CalcMipmapLevelsForReplacement(u32 width, u32 height)
{
	return static_cast<u32>(std::log2(std::max(width, height))) + 1u;
}

GSTexture* GSTextureReplacements::LookupReplacementTexture(const GSTextureCache::HashCacheKey& hash, bool mipmap, bool* pending)
{
	const TextureName name(CreateTextureName(hash, 0));
	*pending = false;

	// replacement for this name exists?
	auto fnit = s_replacement_texture_filenames.find(name);
	if (fnit == s_replacement_texture_filenames.end())
		return nullptr;

	// try the full cache first, to avoid reloading from disk
	{
		std::unique_lock<std::mutex> lock(s_replacement_texture_cache_mutex);
		auto it = s_replacement_texture_cache.find(name);
		if (it != s_replacement_texture_cache.end())
		{
			// replacement is cached, can immediately upload to host GPU
			return CreateReplacementTexture(it->second, name.ReplacementScale(it->second), mipmap);
		}
	}

	// load asynchronously?
	if (GSConfig.LoadTextureReplacementsAsync)
	{
		// replacement will be injected into the TC later on
		std::unique_lock<std::mutex> lock(s_replacement_texture_cache_mutex);
		QueueAsyncReplacementTextureLoad(name, fnit->second, mipmap);

		*pending = true;
		return nullptr;
	}
	else
	{
		// synchronous load
		std::optional<ReplacementTexture> replacement(LoadReplacementTexture(name, fnit->second, !mipmap));
		if (!replacement.has_value())
			return nullptr;

		// insert into cache
		std::unique_lock<std::mutex> lock(s_replacement_texture_cache_mutex);
		const ReplacementTexture& rtex = s_replacement_texture_cache.emplace(name, std::move(replacement.value())).first->second;

		// and upload to gpu
		return CreateReplacementTexture(rtex, name.ReplacementScale(rtex), mipmap);
	}
}

std::optional<GSTextureReplacements::ReplacementTexture> GSTextureReplacements::LoadReplacementTexture(const TextureName& name, const std::string& filename, bool only_base_image)
{
	ReplacementTextureLoader loader = GetLoader(filename);
	if (!loader)
		return std::nullopt;

	ReplacementTexture rtex;
	if (!loader(filename.c_str(), &rtex, only_base_image))
	{
		Console.Warning("Failed to load replacement texture %s", filename.c_str());
		return std::nullopt;
	}

	// decompress formats not supported by the host GPU
	if (!IsFormatSupported(rtex.format))
		DecompressReplacementTexture(rtex);

	return rtex;
}

static bool s_rgbcx_initialized = false;

static void UnpackBC2(const void* block, u32* pixels)
{
	rgbcx::unpack_bc1(block, pixels, false);

	// based on https://github.com/Anteru/dxt-decompress/blob/master/decompress.c#L330
	const u8* block_ptr = reinterpret_cast<const u8*>(block);
	for (u32 i = 0; i < 4; ++i)
	{
		u16 alpha;
		std::memcpy(&alpha, block_ptr, sizeof(alpha));
		block_ptr += sizeof(alpha);

		pixels[i * 4 + 0] = (((alpha) >> 0) & 0xF) * 17;
		pixels[i * 4 + 1] = (((alpha) >> 4) & 0xF) * 17;
		pixels[i * 4 + 2] = (((alpha) >> 8) & 0xF) * 17;
		pixels[i * 4 + 3] = (((alpha) >> 12) & 0xF) * 17;
	}
}

template<GSTexture::Format format>
static void DecompressBC(u32 width, u32 height, u32& pitch, std::vector<u8>& data)
{
	constexpr u32 BC_BLOCK_SIZE = 4;
	constexpr u32 BC_BLOCK_BYTES = 16;

	if (format >= GSTexture::Format::BC1 && format < GSTexture::Format::BC7 && !s_rgbcx_initialized)
	{
		s_rgbcx_initialized = true;
		rgbcx::init();
	}

	const u32 new_pitch = width * sizeof(u32);
	std::vector<u8> new_data(new_pitch * height);
	u32 block_pixels_out[BC_BLOCK_SIZE * BC_BLOCK_SIZE];

	const u32 blocks_wide = (width + (BC_BLOCK_SIZE - 1)) / BC_BLOCK_SIZE;
	const u32 blocks_high = (height + (BC_BLOCK_SIZE - 1)) / BC_BLOCK_SIZE;
	for (u32 y = 0; y < blocks_high; y++)
	{
		const u8* block_in = data.data() + y * pitch;
		for (u32 x = 0; x < blocks_wide; x++, block_in += BC_BLOCK_BYTES)
		{
			// decompress block
			switch (format)
			{
			case GSTexture::Format::BC1:
				rgbcx::unpack_bc1(block_in, block_pixels_out);
				break;
			case GSTexture::Format::BC2:
				UnpackBC2(block_in, block_pixels_out);
				break;
			case GSTexture::Format::BC3:
				rgbcx::unpack_bc3(block_in, block_pixels_out);
				break;
			case GSTexture::Format::BC7:
				bc7decomp::unpack_bc7(block_in, reinterpret_cast<bc7decomp::color_rgba*>(block_pixels_out));
				break;
			}

			// and write it to the new image
			const u32* copy_in_ptr = block_pixels_out;
			u8* copy_out_ptr = new_data.data() + (((y * BC_BLOCK_SIZE) * new_pitch) + (x * BC_BLOCK_SIZE * sizeof(u32)));
			for (u32 sy = 0; sy < 4; sy++)
			{
				std::memcpy(copy_out_ptr, copy_in_ptr, sizeof(u32) * BC_BLOCK_SIZE);
				copy_in_ptr += BC_BLOCK_SIZE;
				copy_out_ptr += new_pitch;
			}
		}
	}

	pitch = new_pitch;
	data = std::move(new_data);
}

static void DecompressTextureLevel(GSTexture::Format format, u32 width, u32 height, u32& pitch, std::vector<u8>& data)
{
	// clang-format off
	switch (format)
	{
	case GSTexture::Format::BC1: DecompressBC<GSTexture::Format::BC1>(width, height, pitch, data); break;
	case GSTexture::Format::BC2: DecompressBC<GSTexture::Format::BC2>(width, height, pitch, data); break;
	case GSTexture::Format::BC3: DecompressBC<GSTexture::Format::BC3>(width, height, pitch, data); break;
	case GSTexture::Format::BC7: DecompressBC<GSTexture::Format::BC7>(width, height, pitch, data); break;
	default: break;
	}
	// clang-format on
}

void GSTextureReplacements::DecompressReplacementTexture(ReplacementTexture& rtex)
{
	if (rtex.format >= GSTexture::Format::BC1 && rtex.format <= GSTexture::Format::BC3 && !s_rgbcx_initialized)
	{
		s_rgbcx_initialized = true;
		rgbcx::init();
	}

	DecompressTextureLevel(rtex.format, rtex.width, rtex.height, rtex.pitch, rtex.data);

	for (u32 mip = 0; mip < static_cast<u32>(rtex.mips.size()); mip++)
	{
		const u32 mip_width = std::max<u32>(rtex.width >> (mip + 1), 1u);
		const u32 mip_height = std::max<u32>(rtex.height >> (mip + 1), 1u);
		DecompressTextureLevel(rtex.format, mip_width, mip_height, rtex.mips[mip].pitch, rtex.mips[mip].data);
	}

	rtex.format = GSTexture::Format::Color;
}

void GSTextureReplacements::QueueAsyncReplacementTextureLoad(const TextureName& name, const std::string& filename, bool mipmap)
{
	// check the pending list, so we don't queue it up multiple times
	if (s_pending_async_load_textures.find(name) != s_pending_async_load_textures.end())
		return;

	s_pending_async_load_textures.insert(name);
	QueueWorkerThreadItem([name, filename, mipmap]() {
		// actually load the file, this is what will take the time
		std::optional<ReplacementTexture> replacement(LoadReplacementTexture(name, filename, !mipmap));

		// check the pending set, there's a race here if we disable replacements while loading otherwise
		std::unique_lock<std::mutex> lock(s_replacement_texture_cache_mutex);
		if (s_pending_async_load_textures.find(name) == s_pending_async_load_textures.end())
			return;

		// insert into the cache and queue for later injection
		if (replacement.has_value())
		{
			s_replacement_texture_cache.emplace(name, std::move(replacement.value()));
			s_async_loaded_textures.emplace_back(name, mipmap);
		}
		else
		{
			// loading failed, so clear it from the pending list
			s_pending_async_load_textures.erase(name);
		}
	});
}

void GSTextureReplacements::PrecacheReplacementTextures()
{
	std::unique_lock<std::mutex> lock(s_replacement_texture_cache_mutex);

	// predict whether the requests will come with mipmaps
	// TODO: This will be wrong for hw mipmap games like Jak.
	const bool mipmap = GSConfig.HWMipmap >= HWMipmapLevel::Basic ||
						GSConfig.UserHacks_TriFilter == TriFiltering::Forced;

	// pretty simple, just go through the filenames and if any aren't cached, cache them
	for (const auto& it : s_replacement_texture_filenames)
	{
		if (s_replacement_texture_cache.find(it.first) != s_replacement_texture_cache.end())
			continue;

		// precaching always goes async.. for now
		QueueAsyncReplacementTextureLoad(it.first, it.second, mipmap);
	}
}

void GSTextureReplacements::ClearReplacementTextures()
{
	s_replacement_texture_filenames.clear();

	std::unique_lock<std::mutex> lock(s_replacement_texture_cache_mutex);
	s_replacement_texture_cache.clear();
	s_pending_async_load_textures.clear();
	s_async_loaded_textures.clear();
}

GSTexture* GSTextureReplacements::CreateReplacementTexture(const ReplacementTexture& rtex, const GSVector2& scale, bool mipmap)
{
	// can't use generated mipmaps with compressed formats, because they can't be rendered to
	// in the future I guess we could decompress the dds and generate them... but there's no reason that modders can't generate mips in dds
	if (mipmap && GSTexture::IsCompressedFormat(rtex.format) && rtex.mips.empty())
	{
		static bool log_once = false;
		if (!log_once)
		{
			Console.Warning("Disabling mipmaps on one or more compressed replacement textures.");
			log_once = true;
		}

		mipmap = false;
	}

	GSTexture* tex = g_gs_device->CreateTexture(rtex.width, rtex.height, mipmap, rtex.format);
	if (!tex)
		return nullptr;

	// upload base level
	tex->Update(GSVector4i(0, 0, rtex.width, rtex.height), rtex.data.data(), rtex.pitch);

	// and the mips if they're present in the replacement texture
	if (!rtex.mips.empty())
	{
		for (u32 i = 0; i < static_cast<u32>(rtex.mips.size()); i++)
		{
			const u32 mip = i + 1;
			const u32 mipw = std::max<u32>(rtex.width >> mip, 1u);
			const u32 miph = std::max<u32>(rtex.height >> mip, 1u);
			tex->Update(GSVector4i(0, 0, mipw, miph), rtex.mips[i].data.data(), rtex.mips[i].pitch, mip);
		}
	}

	tex->SetScale(scale);
	return tex;
}

void GSTextureReplacements::ProcessAsyncLoadedTextures()
{
	// this holds the lock while doing the upload, but it should be reasonably quick
	std::unique_lock<std::mutex> lock(s_replacement_texture_cache_mutex);
	for (const auto& [name, mipmap] : s_async_loaded_textures)
	{
		// no longer pending!
		s_pending_async_load_textures.erase(name);

		// we should be in the cache now, lock and loaded
		auto it = s_replacement_texture_cache.find(name);
		if (it == s_replacement_texture_cache.end())
			continue;

		// upload and inject into TC
		GSTexture* tex = CreateReplacementTexture(it->second, name.ReplacementScale(it->second), mipmap);
		if (tex)
			s_tc->InjectHashCacheTexture(HashCacheKeyFromTextureName(name), tex);
	}
	s_async_loaded_textures.clear();
}

void GSTextureReplacements::DumpTexture(const GSTextureCache::HashCacheKey& hash, const GIFRegTEX0& TEX0, const GIFRegTEXA& TEXA, GSLocalMemory& mem, u32 level)
{
	// check if it's been dumped already
	const TextureName name(CreateTextureName(hash, level));
	if (s_dumped_textures.find(name) != s_dumped_textures.end())
		return;

	s_dumped_textures.insert(name);

	// already exists on disk?
	std::string filename(GetDumpFilename(name, level));
	if (filename.empty() || FileSystem::FileExists(filename.c_str()))
		return;

	const std::string_view title(FileSystem::GetFileTitleFromPath(filename));
	DevCon.WriteLn("Dumping %ux%u texture '%.*s'.", name.Width(), name.Height(), static_cast<int>(title.size()), title.data());

	// compute width/height
	const GSLocalMemory::psm_t& psm = GSLocalMemory::m_psm[TEX0.PSM];
	const GSVector2i& bs = psm.bs;
	const int tw = 1 << TEX0.TW;
	const int th = 1 << TEX0.TH;
	const GSVector4i rect(0, 0, tw, th);
	const GSVector4i block_rect(rect.ralign<Align_Outside>(bs));
	const int read_width = std::max(tw, psm.bs.x);
	const int read_height = std::max(th, psm.bs.y);
	const u32 pitch = static_cast<u32>(read_width) * sizeof(u32);

	// use per-texture buffer so we can compress the texture asynchronously and not block the GS thread
	std::vector<u8> buffer(pitch * static_cast<u32>(read_height));
	(mem.*psm.rtx)(mem.GetOffset(TEX0.TBP0, TEX0.TBW, TEX0.PSM), block_rect, buffer.data(), pitch, TEXA);

	// okay, now we can actually dump it
	QueueWorkerThreadItem([filename = std::move(filename), tw, th, pitch, buffer = std::move(buffer)]() {
		if (!SavePNGImage(filename.c_str(), tw, th, buffer.data(), pitch))
			Console.Error("Failed to dump texture to '%s'.", filename.c_str());
	});
}

void GSTextureReplacements::ClearDumpedTextureList()
{
	s_dumped_textures.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Worker Thread
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GSTextureReplacements::StartWorkerThread()
{
	std::unique_lock<std::mutex> lock(s_worker_thread_mutex);

	if (s_worker_thread.joinable())
		return;

	s_worker_thread_running = true;
	s_worker_thread = std::thread(WorkerThreadEntryPoint);
}

void GSTextureReplacements::StopWorkerThread()
{
	{
		std::unique_lock<std::mutex> lock(s_worker_thread_mutex);
		if (!s_worker_thread.joinable())
			return;

		s_worker_thread_running = false;
		s_worker_thread_cv.notify_one();
	}

	s_worker_thread.join();

	// clear out workery-things too
	CancelPendingLoadsAndDumps();
}

void GSTextureReplacements::QueueWorkerThreadItem(std::function<void()> fn)
{
	pxAssert(s_worker_thread.joinable());

	std::unique_lock<std::mutex> lock(s_worker_thread_mutex);
	s_worker_thread_queue.push(std::move(fn));
	s_worker_thread_cv.notify_one();
}

void GSTextureReplacements::WorkerThreadEntryPoint()
{
	std::unique_lock<std::mutex> lock(s_worker_thread_mutex);
	while (s_worker_thread_running)
	{
		if (s_worker_thread_queue.empty())
		{
			s_worker_thread_cv.wait(lock);
			continue;
		}

		std::function<void()> fn = std::move(s_worker_thread_queue.front());
		s_worker_thread_queue.pop();
		lock.unlock();
		fn();
		lock.lock();
	}
}

void GSTextureReplacements::SyncWorkerThread()
{
	std::unique_lock<std::mutex> lock(s_worker_thread_mutex);
	if (!s_worker_thread.joinable())
		return;

	// not the most efficient by far, but it only gets called on config changes, so whatever
	for (;;)
	{
		if (s_worker_thread_queue.empty())
			break;

		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		lock.lock();
	}
}

void GSTextureReplacements::CancelPendingLoadsAndDumps()
{
	std::unique_lock<std::mutex> lock(s_worker_thread_mutex);
	if (!s_worker_thread.joinable())
		return;

	while (!s_worker_thread_queue.empty())
		s_worker_thread_queue.pop();
	s_async_loaded_textures.clear();
	s_pending_async_load_textures.clear();
}

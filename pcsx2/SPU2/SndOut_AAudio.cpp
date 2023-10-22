/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021  PCSX2 Dev Team
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

#ifdef __ANDROID__

#ifdef __INTELLISENSE__
#include "C:\Users\User\AppData\Local\Android\Sdk\ndk\23.1.7779620\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\include\aaudio\AAudio.h"
#else
#include <aaudio/AAudio.h>
#endif

#include <atomic>

#include "common/ScopedGuard.h"
#include "common/Threading.h"

#include "Global.h"
#include "SndOut.h"
#include "Host.h"
#include "VMManager.h"

struct AAudioMod : public SndOutModule
{
	static constexpr u32 BUFFER_SIZE = 2048;
	static_assert((BUFFER_SIZE % SndOutPacketSize) == 0, "buffer size is multiple of snd out size");

	static AAudioMod mod;

	bool Init() override
	{
		if (!Open())
			return false;

		if (!Start())
		{
			Close();
			return false;
		}

		return true;
	}

	const wchar_t* GetIdent() const override { return L"AAudio"; }
	const wchar_t* GetLongName() const override { return L"Android AAudio"; }

	~AAudioMod()
	{
		Close();
	}

	bool Open()
	{
		AAudioStreamBuilder* builder;
		aaudio_result_t result = AAudio_createStreamBuilder(&builder);
		if (result != AAUDIO_OK)
		{
			Console.Error("AAudio_createStreamBuilder failed: %d", result);
			return false;
		}

		ScopedGuard builder_cleanup([builder]() { AAudioStreamBuilder_delete(builder); });
		AAudioStreamBuilder_setDeviceId(builder, AAUDIO_UNSPECIFIED);
		AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
		AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_SHARED);
		AAudioStreamBuilder_setSampleRate(builder, SampleRate);
		AAudioStreamBuilder_setChannelCount(builder, 2);
		AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
		AAudioStreamBuilder_setBufferCapacityInFrames(builder, BUFFER_SIZE);
		AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
		AAudioStreamBuilder_setFramesPerDataCallback(builder, BUFFER_SIZE);
		AAudioStreamBuilder_setDataCallback(builder, &AAudioMod::DataCallback, nullptr);
		AAudioStreamBuilder_setErrorCallback(builder, &AAudioMod::ErrorCallback, nullptr);

		Console.WriteLn("(AAudioMod) Creating stream...");
		result = AAudioStreamBuilder_openStream(builder, &m_stream);
		if (result != AAUDIO_OK)
		{
			Console.Error("AAudioStreamBuilder_openStream failed: %d", result);
			return false;
		}

		return true;
	}

	bool Start()
	{
		if (m_playing)
			return true;

		Console.WriteLn("(AAudioMod) Starting stream...");
		m_stop_requested = false;

		aaudio_result_t result = AAudioStream_requestStart(m_stream);
		if (result != AAUDIO_OK)
		{
			Console.Error("AAudioStream_requestStart failed: %d", result);
			return false;
		}

		const int64_t timeoutNanos = 100 * 1000000;
		aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
		result = AAudioStream_waitForStateChange(m_stream, AAUDIO_STREAM_STATE_STARTED, &nextState, timeoutNanos);
		if (result != AAUDIO_OK)
			Console.Error("AAudioStream_waitForStateChange on play failed: %d", result);

		m_playing = true;
		m_paused = false;
		return true;
	}

	void Stop()
	{
		if (!m_playing)
			return;

		Console.WriteLn("(AAudioMod) Stopping stream...");
		m_stop_requested = true;

		aaudio_result_t result = AAudioStream_requestStop(m_stream);
		if (result != AAUDIO_OK)
		{
			Console.Error("AAudioStream_requestStop failed: %d", result);
			return;
		}

		const int64_t timeoutNanos = 100 * 1000000;
		aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
		result = AAudioStream_waitForStateChange(m_stream, AAUDIO_STREAM_STATE_STARTED, &nextState, timeoutNanos);
		if (result != AAUDIO_OK)
			Console.Error("(AAudioMod) AAudioStream_waitForStateChange on stop failed: %d", result);

		m_playing = false;
		m_paused = false;
	}

	void Close() override
	{
		Console.WriteLn("(AAudioMod) Closing stream...");
		m_disconnect_counter.store(0);

		if (m_playing)
			Stop();

		if (m_stream)
		{
			AAudioStream_close(m_stream);
			m_stream = nullptr;
		}
	}

	void SetPaused(bool paused) override
	{
		if (paused == m_paused || !m_playing || !m_stream)
			return;

		if (paused)
		{
			Console.WriteLn("(AAudioMod) Requesting pause...");
			aaudio_stream_state_t result = AAudioStream_requestPause(m_stream);
			if (result != AAUDIO_OK)
			{
				Console.Error("AAudioStream_requestPause failed: %d", result);
				return;
			}

			const int64_t timeoutNanos = 100 * 1000000;
			aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
			result = AAudioStream_waitForStateChange(m_stream, AAUDIO_STREAM_STATE_STARTED, &nextState, timeoutNanos);
			if (result != AAUDIO_OK)
				Console.Error("(AAudioMod) AAudioStream_waitForStateChange on pause failed: %d", result);
		}
		else
		{
			Console.WriteLn("(AAudioMod) Requesting restart after pause...");

			aaudio_stream_state_t result = AAudioStream_requestStart(m_stream);
			if (result != AAUDIO_OK)
			{
				Console.Error("AAudioStream_requestStart failed: %d", result);
				return;
			}

			const int64_t timeoutNanos = 100 * 1000000;
			aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_UNINITIALIZED;
			result = AAudioStream_waitForStateChange(m_stream, AAUDIO_STREAM_STATE_PAUSED, &nextState, timeoutNanos);
			if (result != AAUDIO_OK)
				Console.Error("(AAudioMod) AAudioStream_waitForStateChange on unpause start failed: %d", result);
		}

		m_paused = paused;
	}

	static aaudio_data_callback_result_t DataCallback(AAudioStream* stream, void* userData, void* audioData, int32_t numFrames)
	{
		StereoOut16* out = (StereoOut16*)audioData;
		pxAssertRel((numFrames & SndOutPacketSize) == 0, "Packet size is aligned");
		for (int32_t i = 0; i < numFrames; i += SndOutPacketSize)
			SndBuffer::ReadSamples(&out[i]);

		return AAUDIO_CALLBACK_RESULT_CONTINUE;
	}

	static void ErrorCallback(AAudioStream* stream, void* userData, aaudio_result_t error)
	{
		Console.Error("(AAudioMod) AAudio ErrorCB %d", error);
		if (error == AAUDIO_ERROR_DISCONNECTED && !mod.m_stop_requested)
		{
			Console.Error("(AAudioMod) Audio stream disconnected, queueing reopening...");

			const u32 counter = mod.m_disconnect_counter.fetch_add(1u) + 1u;

			Host::RunOnCPUThread([counter]() {
				if (counter != mod.m_disconnect_counter.load() || !mod.m_stream)
				{
					Console.WriteLn("(AAudioMod) Counter mismatch, ignoring reopen");
					return;
				}

				mod.Restart();
			});
		}
	}

	void Restart()
	{
		const bool was_paused = m_paused;

		Console.WriteLn("(AAudioMod) Reopening audio stream...");
		Stop();
		Close();
		if (Open())
		{
			// try 5 times to reopen the stream... android seems to like to throw random errors here.
			constexpr u32 RETRY_ATTEMPTS = 5;
			for (u32 retry_attempt = 0; retry_attempt < RETRY_ATTEMPTS; retry_attempt++)
			{
				if (mod.Start())
				{
					SetPaused(was_paused);
					return;
				}

				Console.WriteLn("(AAudioMod) Failed to restart stream after disconnection on the %uth try, sleeping and trying again...", retry_attempt + 1u);
				Threading::Sleep(10);
			}

			Host::ReportFormattedErrorAsync("AAudioMod", "Failed to restart audio stream after %u attempts.", RETRY_ATTEMPTS);
		}
		else
		{
			Console.Error("(AAudioMod) Failed to reopen stream after disconnection.");
		}
	}


	int GetEmptySampleCount() override
	{
		const u64 pos = AAudioStream_getFramesRead(m_stream);

		int playedSinceLastTime = (writtenSoFar - writtenLastTime) + (pos - positionLastTime);
		writtenLastTime = writtenSoFar;
		positionLastTime = pos;

		// Lowest resolution here is the SndOutPacketSize we use.
		return playedSinceLastTime;
	}

private:
	AAudioStream* m_stream = nullptr;
	std::atomic_uint32_t m_disconnect_counter{0};
	std::atomic_bool m_stop_requested{false};
	bool m_playing = false;
	bool m_paused = false;

	u64 writtenSoFar = 0;
	u64 writtenLastTime = 0;
	u64 positionLastTime = 0;

	AAudioMod() = default;
};

AAudioMod AAudioMod::mod;

SndOutModule* const AAudioOut = &AAudioMod::mod;

#endif
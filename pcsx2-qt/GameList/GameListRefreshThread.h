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

#include <QtCore/QThread>
#include <QtCore/QSemaphore>

#include "common/ProgressCallback.h"
#include "common/Timer.h"

class GameListRefreshThread;

class AsyncRefreshProgressCallback : public BaseProgressCallback
{
public:
	AsyncRefreshProgressCallback(GameListRefreshThread* parent);

	void Cancel();

	void SetStatusText(const char* text) override;
	void SetProgressRange(u32 range) override;
	void SetProgressValue(u32 value) override;
	void SetTitle(const char* title) override;
	void DisplayError(const char* message) override;
	void DisplayWarning(const char* message) override;
	void DisplayInformation(const char* message) override;
	void DisplayDebugMessage(const char* message) override;
	void ModalError(const char* message) override;
	bool ModalConfirmation(const char* message) override;
	void ModalInformation(const char* message) override;

private:
	void fireUpdate();

	GameListRefreshThread* m_parent;
	Common::Timer m_last_update_time;
	QString m_status_text;
	int m_last_range = 1;
	int m_last_value = 0;
};

class GameListRefreshThread final : public QThread
{
	Q_OBJECT

public:
	GameListRefreshThread(bool invalidate_cache);
	~GameListRefreshThread();

	void cancel();

Q_SIGNALS:
	void refreshProgress(const QString& status, int current, int total);
	void refreshComplete();

protected:
	void run();

private:
	AsyncRefreshProgressCallback m_progress;
	bool m_invalidate_cache;
};

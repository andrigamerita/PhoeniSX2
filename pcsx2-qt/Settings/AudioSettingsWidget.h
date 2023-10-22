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

#include <QtWidgets/QWidget>

#include "ui_AudioSettingsWidget.h"

class SettingsDialog;

class AudioSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	AudioSettingsWidget(SettingsDialog* dialog, QWidget* parent);
	~AudioSettingsWidget();

private Q_SLOTS:
	void updateVolumeLabel();
	void updateLatencyLabel();
	void updateTimestretchSequenceLengthLabel();
	void updateTimestretchSeekwindowLengthLabel();
	void updateTimestretchOverlapLabel();
	void resetTimestretchDefaults();

private:
	Ui::AudioSettingsWidget m_ui;
};

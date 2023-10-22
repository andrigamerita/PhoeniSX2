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

#include "ui_EmulationSettingsWidget.h"

class SettingsDialog;

class EmulationSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	EmulationSettingsWidget(SettingsDialog* dialog, QWidget* parent);
	~EmulationSettingsWidget();

private Q_SLOTS:
	void onOptimalFramePacingChanged();

private:
	void initializeSpeedCombo(QComboBox* cb, const char* section, const char* key, float default_value);
	void handleSpeedComboChange(QComboBox* cb, const char* section, const char* key);
	void updateOptimalFramePacing();

	SettingsDialog* m_dialog;

	Ui::EmulationSettingsWidget m_ui;
};

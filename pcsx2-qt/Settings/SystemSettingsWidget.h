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

#include "ui_SystemSettingsWidget.h"

class SettingsDialog;

class SystemSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	SystemSettingsWidget(SettingsDialog* dialog, QWidget* parent);
	~SystemSettingsWidget();

private Q_SLOTS:
	void updateVU1InstantState();

private:
	int getGlobalClampingModeIndex(bool vu) const;
	int getClampingModeIndex(bool vu) const;
	void setClampingMode(bool vu, int index);

	SettingsDialog* m_dialog;

	Ui::SystemSettingsWidget m_ui;
};

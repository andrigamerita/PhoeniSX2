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

#include "ui_GameFixSettingsWidget.h"

class SettingsDialog;

class GameFixSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	GameFixSettingsWidget(SettingsDialog* dialog, QWidget* parent);
	~GameFixSettingsWidget();

private:
	Ui::GameFixSettingsWidget m_ui;
};

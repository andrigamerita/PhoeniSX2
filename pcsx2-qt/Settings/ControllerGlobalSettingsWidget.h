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
#include <QtCore/QMap>
#include <array>
#include <vector>

#include "ui_ControllerGlobalSettingsWidget.h"

class ControllerSettingsDialog;

class ControllerGlobalSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	ControllerGlobalSettingsWidget(QWidget* parent, ControllerSettingsDialog* dialog);
	~ControllerGlobalSettingsWidget();

	void addDeviceToList(const QString& identifier, const QString& name);
	void removeDeviceFromList(const QString& identifier);

private:
	void updateSDLOptionsEnabled();

	Ui::ControllerGlobalSettingsWidget m_ui;
};

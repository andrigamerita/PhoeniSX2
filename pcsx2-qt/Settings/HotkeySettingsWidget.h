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

class QTabWidget;
class QGridLayout;

class ControllerSettingsDialog;

class HotkeySettingsWidget : public QWidget
{
	Q_OBJECT

public:
	HotkeySettingsWidget(QWidget* parent, ControllerSettingsDialog* dialog);
	~HotkeySettingsWidget();

private:
	void createUi();
	void createButtons();

	QTabWidget* m_tab_widget;

	struct Category
	{
		QWidget* container;
		QGridLayout* layout;
	};
	QMap<QString, Category> m_categories;
};

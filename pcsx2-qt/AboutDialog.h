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

#include "ui_AboutDialog.h"
#include <QtWidgets/QDialog>

class AboutDialog final : public QDialog
{
	Q_OBJECT

public:
	explicit AboutDialog(QWidget* parent = nullptr);
	~AboutDialog();

	static QString getWebsiteUrl();
	static QString getDiscordServerUrl();
	static QString getPatreonUrl();

private:
	Ui::AboutDialog m_ui;
};

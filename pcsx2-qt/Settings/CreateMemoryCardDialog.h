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

#include <QtWidgets/QDialog>

#include "ui_CreateMemoryCardDialog.h"

#include "pcsx2/Config.h"

class CreateMemoryCardDialog final : public QDialog
{
	Q_OBJECT

public:
	explicit CreateMemoryCardDialog(QWidget* parent = nullptr);
	~CreateMemoryCardDialog();

private Q_SLOTS:
	void nameTextChanged();
	void createCard();

private:
	void setType(MemoryCardType type, MemoryCardFileType fileType);
	void restoreDefaults();
	void updateState();

	Ui::CreateMemoryCardDialog m_ui;

	MemoryCardType m_type = MemoryCardType::File;
	MemoryCardFileType m_fileType = MemoryCardFileType::PS2_8MB;
};

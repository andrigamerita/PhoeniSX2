/*  AetherSX2 - PS2 Emulator for Android and ARM PCs
 *  Copyright (C) 2022 AetherSX2 Dev Team
 *
 *  AetherSX2 is provided under the terms of the Creative Commons
 *  Attribution-NonCommercial-NoDerivatives International License
 *  (BY-NC-ND 4.0, https://creativecommons.org/licenses/by-nc-nd/4.0/).
 * 
 *  Commercialization of this application and source code is forbidden.
 */

#include "PrecompiledHeader.h"

#include "common/StringUtil.h"

#include "Frontend/GameList.h"

#include "GameSummaryWidget.h"

GameSummaryWidget::GameSummaryWidget(const GameList::Entry* entry, SettingsDialog* dialog, QWidget* parent)
{
	m_ui.setupUi(this);
	populateUi(entry);
}

GameSummaryWidget::~GameSummaryWidget() = default;

void GameSummaryWidget::populateUi(const GameList::Entry* entry)
{
	m_ui.title->setText(QString::fromStdString(entry->title));
	m_ui.path->setText(QString::fromStdString(entry->path));
	m_ui.serial->setText(QString::fromStdString(entry->serial));
	m_ui.crc->setText(QString::fromStdString(StringUtil::StdStringFromFormat("%08X", entry->crc)));
	m_ui.type->setCurrentIndex(static_cast<int>(entry->type));
	m_ui.region->setCurrentIndex(static_cast<int>(entry->region));
	m_ui.compatibility->setCurrentIndex(static_cast<int>(entry->compatibility_rating));
}

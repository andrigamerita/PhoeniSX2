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
#include <string>
#include <QtWidgets/QWidget>

#include "ui_GameListSettingsWidget.h"

class SettingsDialog;

class GameListSearchDirectoriesModel;

class GameListSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	GameListSettingsWidget(SettingsDialog* dialog, QWidget* parent);
	~GameListSettingsWidget();

	bool addExcludedPath(const std::string& path);
	void refreshExclusionList();

public Q_SLOTS:
	void addSearchDirectory(QWidget* parent_widget);

private Q_SLOTS:
	void onDirectoryListContextMenuRequested(const QPoint& point);
	void onAddSearchDirectoryButtonClicked();
	void onRemoveSearchDirectoryButtonClicked();
	void onAddExcludedPathButtonClicked();
	void onRemoveExcludedPathButtonClicked();
	void onScanForNewGamesClicked();
	void onRescanAllGamesClicked();

protected:
	void resizeEvent(QResizeEvent* event);

private:
	void addPathToTable(const std::string& path, bool recursive);
	void refreshDirectoryList();
	void addSearchDirectory(const QString& path, bool recursive);
	void removeSearchDirectory(const QString& path);

	Ui::GameListSettingsWidget m_ui;
};

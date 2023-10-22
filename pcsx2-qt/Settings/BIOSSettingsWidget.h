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
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtCore/QVector>
#include <QtWidgets/QWidget>
#include <string>

#include "ui_BIOSSettingsWidget.h"

class SettingsDialog;
class QThread;

// TODO: Move to core.
struct BIOSInfo
{
	std::string filename;
	std::string description;
	std::string zone;
	u32 version;
	u32 region;
};
Q_DECLARE_METATYPE(BIOSInfo);

class BIOSSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	BIOSSettingsWidget(SettingsDialog* dialog, QWidget* parent);
	~BIOSSettingsWidget();

private Q_SLOTS:
	void refreshList();
	void browseSearchDirectory();
	void openSearchDirectory();
	void updateSearchDirectory();

	void listItemChanged(const QTreeWidgetItem* current, const QTreeWidgetItem* previous);
	void listRefreshed(const QVector<BIOSInfo>& items);

private:
	Ui::BIOSSettingsWidget m_ui;

	class RefreshThread final : public QThread
	{
	public:
		RefreshThread(BIOSSettingsWidget* parent, const QString& directory);
		~RefreshThread();

	protected:
		void run() override;

	private:
		BIOSSettingsWidget* m_parent;
		QString m_directory;
	};

	RefreshThread* m_refresh_thread = nullptr;
};

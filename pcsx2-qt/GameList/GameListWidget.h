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
#include "pcsx2/Frontend/GameList.h"
#include <QtWidgets/QListView>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableView>

Q_DECLARE_METATYPE(const GameList::Entry*);

class GameListModel;
class GameListSortModel;
class GameListRefreshThread;

class GameListGridListView : public QListView
{
	Q_OBJECT

public:
	GameListGridListView(QWidget* parent = nullptr);

Q_SIGNALS:
	void zoomOut();
	void zoomIn();

protected:
	void wheelEvent(QWheelEvent* e);
};

class GameListWidget : public QStackedWidget
{
	Q_OBJECT

public:
	GameListWidget(QWidget* parent = nullptr);
	~GameListWidget();

	__fi GameListModel* getModel() const { return m_model; }

	void initialize();

	void refresh(bool invalidate_cache);

	bool isShowingGameList() const;
	bool isShowingGameGrid() const;

	bool getShowGridCoverTitles() const;

	const GameList::Entry* getSelectedEntry() const;

Q_SIGNALS:
	void refreshProgress(const QString& status, int current, int total);
	void refreshComplete();

	void selectionChanged();
	void entryActivated();
	void entryContextMenuRequested(const QPoint& point);

private Q_SLOTS:
	void onRefreshProgress(const QString& status, int current, int total);
	void onRefreshComplete();

	void onSelectionModelCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
	void onTableViewItemActivated(const QModelIndex& index);
	void onTableViewContextMenuRequested(const QPoint& point);
	void onTableViewHeaderContextMenuRequested(const QPoint& point);
	void onTableViewHeaderSortIndicatorChanged(int, Qt::SortOrder);
	void onListViewItemActivated(const QModelIndex& index);
	void onListViewContextMenuRequested(const QPoint& point);

public Q_SLOTS:
	void showGameList();
	void showGameGrid();
	void setShowCoverTitles(bool enabled);
	void gridZoomIn();
	void gridZoomOut();
	void refreshGridCovers();

protected:
	void resizeEvent(QResizeEvent* event);

private:
	void resizeTableViewColumnsToFit();
	void loadTableViewColumnVisibilitySettings();
	void saveTableViewColumnVisibilitySettings();
	void saveTableViewColumnVisibilitySettings(int column);
	void loadTableViewColumnSortSettings();
	void saveTableViewColumnSortSettings();
	void listZoom(float delta);
	void updateListFont();

	GameListModel* m_model = nullptr;
	GameListSortModel* m_sort_model = nullptr;
	QTableView* m_table_view = nullptr;
	GameListGridListView* m_list_view = nullptr;

	GameListRefreshThread* m_refresh_thread = nullptr;
};

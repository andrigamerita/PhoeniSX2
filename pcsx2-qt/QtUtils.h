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
#include <QtCore/QByteArray>
#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <functional>
#include <initializer_list>
#include <string_view>
#include <optional>

class ByteStream;

class QAction;
class QComboBox;
class QFrame;
class QKeyEvent;
class QTableView;
class QTreeView;
class QVariant;
class QWidget;
class QUrl;

// TODO: Get rid of wx interoperability later on.
#include <wx/string.h>

namespace QtUtils
{
	/// Marks an action as the "default" - i.e. makes the text bold.
	void MarkActionAsDefault(QAction* action);

	/// Creates a horizontal line widget.
	QFrame* CreateHorizontalLine(QWidget* parent);

	/// Returns the greatest parent of a widget, i.e. its dialog/window.
	QWidget* GetRootWidget(QWidget* widget, bool stop_at_window_or_dialog = true);

	/// Resizes columns of the table view to at the specified widths. A negative width will stretch the column to use the
	/// remaining space.
	void ResizeColumnsForTableView(QTableView* view, const std::initializer_list<int>& widths);
	void ResizeColumnsForTreeView(QTreeView* view, const std::initializer_list<int>& widths);

	/// Returns a string identifier for a Qt key ID.
	QString GetKeyIdentifier(int key);

	/// Returns the integer Qt key ID for an identifier.
	std::optional<int> GetKeyIdForIdentifier(const QString& key_identifier);

	/// Stringizes a key event.
	QString KeyEventToString(int key, Qt::KeyboardModifiers mods);

	/// Returns an integer id for a stringized key event. Modifiers are in the upper bits.
	std::optional<int> ParseKeyString(const QString& key_str);

	/// Returns a key id for a key event, including any modifiers.
	int KeyEventToInt(int key, Qt::KeyboardModifiers mods);

	/// Opens a URL with the default handler.
	void OpenURL(QWidget* parent, const QUrl& qurl);

	/// Opens a URL string with the default handler.
	void OpenURL(QWidget* parent, const char* url);

	/// Opens a URL string with the default handler.
	void OpenURL(QWidget* parent, const QString& url);

	/// Converts a std::string_view to a QString safely.
	QString StringViewToQString(const std::string_view& str);

	// TODO: Get rid of wx interoperability later on.
	wxString QStringToWxString(const QString& str);
	QString WxStringToQString(const wxString& str);
} // namespace QtUtils
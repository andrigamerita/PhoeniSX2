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
#include "ui_ControllerSettingsDialog.h"
#include "Frontend/InputManager.h"
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QDialog>
#include <array>

class ControllerGlobalSettingsWidget;
class ControllerBindingWidget;
class HotkeySettingsWidget;

class ControllerSettingsDialog final : public QDialog
{
	Q_OBJECT

public:
	enum class Category
	{
		GlobalSettings,
		FirstControllerSettings,
		HotkeySettings,
		Count
	};

	enum : u32
	{
		MAX_PORTS = 2
	};

	ControllerSettingsDialog(QWidget* parent = nullptr);
	~ControllerSettingsDialog();

	HotkeySettingsWidget* getHotkeySettingsWidget() const { return m_hotkey_settings; }

	__fi const QList<QPair<QString, QString>>& getDeviceList() const { return m_device_list; }
	__fi const QStringList& getVibrationMotors() const { return m_vibration_motors; }

public Q_SLOTS:
	void setCategory(Category category);

private Q_SLOTS:
	void onCategoryCurrentRowChanged(int row);

	void onInputDevicesEnumerated(const QList<QPair<QString, QString>>& devices);
	void onInputDeviceConnected(const QString& identifier, const QString& device_name);
	void onInputDeviceDisconnected(const QString& identifier);
	void onVibrationMotorsEnumerated(const QList<InputBindingKey>& motors);

private:
	Ui::ControllerSettingsDialog m_ui;

	ControllerGlobalSettingsWidget* m_global_settings = nullptr;
	std::array<ControllerBindingWidget*, MAX_PORTS> m_port_bindings{};
	HotkeySettingsWidget* m_hotkey_settings = nullptr;

	QList<QPair<QString, QString>> m_device_list;
	QStringList m_vibration_motors;
};

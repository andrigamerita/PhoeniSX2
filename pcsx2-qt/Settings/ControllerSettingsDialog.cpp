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

#include "EmuThread.h"
#include "QtHost.h"
#include "Settings/ControllerSettingsDialog.h"
#include "Settings/ControllerGlobalSettingsWidget.h"
#include "Settings/ControllerBindingWidgets.h"
#include "Settings/HotkeySettingsWidget.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTextEdit>

ControllerSettingsDialog::ControllerSettingsDialog(QWidget* parent /* = nullptr */)
	: QDialog(parent)
{
	m_ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_global_settings = new ControllerGlobalSettingsWidget(m_ui.settingsContainer, this);
	m_ui.settingsContainer->insertWidget(0, m_global_settings);

	for (u32 i = 0; i < MAX_PORTS; i++)
	{
		m_port_bindings[i] = new ControllerBindingWidget(m_ui.settingsContainer, this, i);
		m_ui.settingsContainer->insertWidget(i + 1, m_port_bindings[i]);
	}

	m_hotkey_settings = new HotkeySettingsWidget(m_ui.settingsContainer, this);
	m_ui.settingsContainer->insertWidget(3, m_hotkey_settings);

	m_ui.settingsCategory->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	connect(m_ui.settingsCategory, &QListWidget::currentRowChanged, this, &ControllerSettingsDialog::onCategoryCurrentRowChanged);
	connect(m_ui.buttonBox, &QDialogButtonBox::rejected, this, &ControllerSettingsDialog::close);

	connect(g_emu_thread, &EmuThread::onInputDevicesEnumerated, this, &ControllerSettingsDialog::onInputDevicesEnumerated);
	connect(g_emu_thread, &EmuThread::onInputDeviceConnected, this, &ControllerSettingsDialog::onInputDeviceConnected);
	connect(g_emu_thread, &EmuThread::onInputDeviceDisconnected, this, &ControllerSettingsDialog::onInputDeviceDisconnected);
	connect(g_emu_thread, &EmuThread::onVibrationMotorsEnumerated, this, &ControllerSettingsDialog::onVibrationMotorsEnumerated);

	// trigger a device enumeration to populate the device list
	g_emu_thread->enumerateInputDevices();
	g_emu_thread->enumerateVibrationMotors();
}

ControllerSettingsDialog::~ControllerSettingsDialog() = default;

void ControllerSettingsDialog::setCategory(Category category)
{
	switch (category)
	{
		case Category::GlobalSettings:
			m_ui.settingsCategory->setCurrentRow(0);
			break;

			// TODO: These will need to take multitap into consideration in the future.
		case Category::FirstControllerSettings:
			m_ui.settingsCategory->setCurrentRow(1);
			break;

		case Category::HotkeySettings:
			m_ui.settingsCategory->setCurrentRow(3);
			break;

		default:
			break;
	}
}

void ControllerSettingsDialog::onCategoryCurrentRowChanged(int row)
{
	m_ui.settingsContainer->setCurrentIndex(row);
}

void ControllerSettingsDialog::onInputDevicesEnumerated(const QList<QPair<QString, QString>>& devices)
{
	m_device_list = devices;
	for (const QPair<QString, QString>& device : devices)
		m_global_settings->addDeviceToList(device.first, device.second);
}

void ControllerSettingsDialog::onInputDeviceConnected(const QString& identifier, const QString& device_name)
{
	m_device_list.emplace_back(identifier, device_name);
	m_global_settings->addDeviceToList(identifier, device_name);
	g_emu_thread->enumerateVibrationMotors();
}

void ControllerSettingsDialog::onInputDeviceDisconnected(const QString& identifier)
{
	for (auto iter = m_device_list.begin(); iter != m_device_list.end(); ++iter)
	{
		if (iter->first == identifier)
		{
			m_device_list.erase(iter);
			break;
		}
	}

	m_global_settings->removeDeviceFromList(identifier);
	g_emu_thread->enumerateVibrationMotors();
}

void ControllerSettingsDialog::onVibrationMotorsEnumerated(const QList<InputBindingKey>& motors)
{
	m_vibration_motors.clear();
	m_vibration_motors.reserve(motors.size());

	for (const InputBindingKey key : motors)
	{
		const std::string key_str(InputManager::ConvertInputBindingKeyToString(key));
		if (!key_str.empty())
			m_vibration_motors.push_back(QString::fromStdString(key_str));
	}
}

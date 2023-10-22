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

#include "Frontend/InputManager.h"
#include "Settings/ControllerGlobalSettingsWidget.h"
#include "Settings/ControllerSettingsDialog.h"
#include "QtUtils.h"
#include "SettingWidgetBinder.h"

ControllerGlobalSettingsWidget::ControllerGlobalSettingsWidget(QWidget* parent, ControllerSettingsDialog* dialog)
	: QWidget(parent)
{
	m_ui.setupUi(this);

	SettingWidgetBinder::BindWidgetToBoolSetting(nullptr, m_ui.enableSDLSource, "InputSources", "SDL", true);
	SettingWidgetBinder::BindWidgetToBoolSetting(nullptr, m_ui.enableSDLEnhancedMode, "InputSources", "SDLControllerEnhancedMode", false);
	SettingWidgetBinder::BindWidgetToBoolSetting(nullptr, m_ui.enableXInputSource, "InputSources", "XInput", false);

	connect(m_ui.enableSDLSource, &QCheckBox::stateChanged, this, &ControllerGlobalSettingsWidget::updateSDLOptionsEnabled);
	updateSDLOptionsEnabled();
}

ControllerGlobalSettingsWidget::~ControllerGlobalSettingsWidget() = default;

void ControllerGlobalSettingsWidget::addDeviceToList(const QString& identifier, const QString& name)
{
	QListWidgetItem* item = new QListWidgetItem();
	item->setText(QStringLiteral("%1: %2").arg(identifier).arg(name));
	item->setData(Qt::UserRole, identifier);
	m_ui.deviceList->addItem(item);
}

void ControllerGlobalSettingsWidget::removeDeviceFromList(const QString& identifier)
{
	const int count = m_ui.deviceList->count();
	for (int i = 0; i < count; i++)
	{
		QListWidgetItem* item = m_ui.deviceList->item(i);
		if (item->data(Qt::UserRole) != identifier)
			continue;

		delete m_ui.deviceList->takeItem(i);
		break;
	}
}

void ControllerGlobalSettingsWidget::updateSDLOptionsEnabled()
{
	const bool enabled = m_ui.enableSDLSource->isChecked();
	m_ui.enableSDLEnhancedMode->setEnabled(enabled);
}

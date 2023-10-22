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

#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <algorithm>

#include "ControllerBindingWidgets.h"
#include "ControllerSettingsDialog.h"
#include "EmuThread.h"
#include "QtUtils.h"
#include "SettingWidgetBinder.h"
#include "SettingsDialog.h"

#include "common/StringUtil.h"
#include "pcsx2/PAD/Host/PAD.h"

#include "SettingWidgetBinder.h"

ControllerBindingWidget::ControllerBindingWidget(QWidget* parent, ControllerSettingsDialog* dialog, u32 port)
	: QWidget(parent)
	, m_dialog(dialog)
	, m_config_section(StringUtil::StdStringFromFormat("Pad%u", port + 1u))
	, m_port_number(port)
{
	m_ui.setupUi(this);
	populateControllerTypes();
	onTypeChanged();

	SettingWidgetBinder::BindWidgetToStringSetting(nullptr, m_ui.controllerType, m_config_section, "Type", "None");
	connect(m_ui.controllerType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ControllerBindingWidget::onTypeChanged);
	connect(m_ui.automaticBinding, &QPushButton::clicked, this, &ControllerBindingWidget::doAutomaticBinding);
}

ControllerBindingWidget::~ControllerBindingWidget() = default;

void ControllerBindingWidget::populateControllerTypes()
{
	m_ui.controllerType->addItem(tr("None (Not Connected)"), QStringLiteral("None"));
	for (const std::string& type : PAD::GetControllerTypeNames())
		m_ui.controllerType->addItem(QString::fromStdString(type), QString::fromStdString(type));
}

void ControllerBindingWidget::onTypeChanged()
{
	if (m_current_widget)
	{
		m_ui.verticalLayout->removeWidget(m_current_widget);
		delete m_current_widget;
		m_current_widget = nullptr;
	}

	m_controller_type = QtHost::GetBaseStringSettingValue(m_config_section.c_str(), "Type");

	const int index = m_ui.controllerType->findData(QString::fromStdString(m_controller_type));
	if (index >= 0 && index != m_ui.controllerType->currentIndex())
	{
		QSignalBlocker sb(m_ui.controllerType);
		m_ui.controllerType->setCurrentIndex(index);
	}

	if (m_controller_type == "DualShock2")
		m_current_widget = ControllerBindingWidget_DualShock2::createInstance(this);
	else
		m_current_widget = new ControllerBindingWidget_Base(this);

	m_ui.verticalLayout->addWidget(m_current_widget, 1);
}

void ControllerBindingWidget::doAutomaticBinding()
{
	QMenu menu(this);
	bool added = false;

	for (const QPair<QString, QString>& dev : m_dialog->getDeviceList())
	{
		// we set it as data, because the device list could get invalidated while the menu is up
		QAction* action = menu.addAction(QStringLiteral("%1 (%2)").arg(dev.first).arg(dev.second));
		action->setData(dev.first);
		connect(action, &QAction::triggered, this, [this, action]() {
			doDeviceAutomaticBinding(action->data().toString());
		});
		added = true;
	}

	if (!added)
	{
		QAction* action = menu.addAction(tr("No devices available"));
		action->setEnabled(false);
	}

	menu.exec(QCursor::pos());
}

void ControllerBindingWidget::doDeviceAutomaticBinding(const QString& device)
{
	std::vector<std::pair<GenericInputBinding, std::string>> mapping = InputManager::GetGenericBindingMapping(device.toStdString());
	if (mapping.empty())
	{
		QMessageBox::critical(QtUtils::GetRootWidget(this), tr("Automatic Binding"),
			tr("No generic bindings were generated for device '%1'").arg(device));
		return;
	}

	bool result;
	{
		auto lock = Host::GetSettingsLock();
		result = PAD::MapController(*QtHost::GetBaseSettingsInterface(), m_port_number, mapping);
	}

	if (result)
	{
		// force a refresh after mapping
		onTypeChanged();
		QtHost::QueueSettingsSave();
		g_emu_thread->applySettings();
	}
}

//////////////////////////////////////////////////////////////////////////

ControllerBindingWidget_Base::ControllerBindingWidget_Base(ControllerBindingWidget* parent)
	: QWidget(parent)
{
}

ControllerBindingWidget_Base::~ControllerBindingWidget_Base()
{
}

void ControllerBindingWidget_Base::initBindingWidgets()
{
	const std::string& type = getControllerType();
	const std::string& config_section = getConfigSection();
	std::vector<std::string> bindings(PAD::GetControllerBinds(type));

	for (std::string& binding : bindings)
	{
		InputBindingWidget* widget = findChild<InputBindingWidget*>(QString::fromStdString(binding));
		if (!widget)
		{
			Console.Error("(ControllerBindingWidget_Base) No widget found for '%s' (%.*s)",
				binding.c_str(), static_cast<int>(type.size()), type.data());
			continue;
		}

		widget->setKey(config_section, std::move(binding));
	}

	const PAD::VibrationCapabilities vibe_caps = PAD::GetControllerVibrationCapabilities(type);
	switch (vibe_caps)
	{
		case PAD::VibrationCapabilities::LargeSmallMotors:
		{
			InputVibrationBindingWidget* widget = findChild<InputVibrationBindingWidget*>(QStringLiteral("LargeMotor"));
			if (widget)
				widget->setKey(getDialog(), config_section, "LargeMotor");

			widget = findChild<InputVibrationBindingWidget*>(QStringLiteral("SmallMotor"));
			if (widget)
				widget->setKey(getDialog(), config_section, "SmallMotor");
		}
		break;

		case PAD::VibrationCapabilities::SingleMotor:
		{
			InputVibrationBindingWidget* widget = findChild<InputVibrationBindingWidget*>(QStringLiteral("Motor"));
			if (widget)
				widget->setKey(getDialog(), config_section, "Motor");
		}
		break;

		case PAD::VibrationCapabilities::NoVibration:
		default:
			break;
	}

	if (QSlider* widget = findChild<QSlider*>(QStringLiteral("AxisScale")); widget)
	{
		// position 1.0f at the halfway point
		const float range = static_cast<float>(widget->maximum()) * 0.5f;
		QLabel* label = findChild<QLabel*>(QStringLiteral("AxisScaleLabel"));
		if (label)
		{
			connect(widget, &QSlider::valueChanged, this, [range, label](int value) {
				label->setText(tr("%1x").arg(static_cast<float>(value) / range, 0, 'f', 2));
			});
		}

		SettingWidgetBinder::BindWidgetToNormalizedSetting(nullptr, widget, config_section, "AxisScale", range, 1.0f);
	}

	if (QDoubleSpinBox* widget = findChild<QDoubleSpinBox*>(QStringLiteral("SmallMotorScale")); widget)
		SettingWidgetBinder::BindWidgetToFloatSetting(nullptr, widget, config_section, "SmallMotorScale", 1.0f);
	if (QDoubleSpinBox* widget = findChild<QDoubleSpinBox*>(QStringLiteral("LargeMotorScale")); widget)
		SettingWidgetBinder::BindWidgetToFloatSetting(nullptr, widget, config_section, "LargeMotorScale", 1.0f);
}

ControllerBindingWidget_DualShock2::ControllerBindingWidget_DualShock2(ControllerBindingWidget* parent)
	: ControllerBindingWidget_Base(parent)
{
	m_ui.setupUi(this);
	initBindingWidgets();
}

ControllerBindingWidget_DualShock2::~ControllerBindingWidget_DualShock2()
{
}

ControllerBindingWidget_Base* ControllerBindingWidget_DualShock2::createInstance(ControllerBindingWidget* parent)
{
	return new ControllerBindingWidget_DualShock2(parent);
}

//////////////////////////////////////////////////////////////////////////

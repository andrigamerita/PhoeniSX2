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

#include <QtWidgets/QWidget>

#include "ui_ControllerBindingWidget.h"
#include "ui_ControllerBindingWidget_DualShock2.h"

class InputBindingWidget;
class ControllerSettingsDialog;
class ControllerBindingWidget_Base;

class ControllerBindingWidget final : public QWidget
{
	Q_OBJECT

public:
	ControllerBindingWidget(QWidget* parent, ControllerSettingsDialog* dialog, u32 port);
	~ControllerBindingWidget();

	__fi ControllerSettingsDialog* getDialog() const { return m_dialog; }
	__fi const std::string& getConfigSection() const { return m_config_section; }
	__fi const std::string& getControllerType() const { return m_controller_type; }
	__fi u32 getPortNumber() const { return m_port_number; }

private Q_SLOTS:
	void onTypeChanged();
	void doAutomaticBinding();

private:
	void populateControllerTypes();
	void doDeviceAutomaticBinding(const QString& device);

	Ui::ControllerBindingWidget m_ui;

	ControllerSettingsDialog* m_dialog;

	std::string m_config_section;
	std::string m_controller_type;
	u32 m_port_number;

	ControllerBindingWidget_Base* m_current_widget = nullptr;
};

class ControllerBindingWidget_Base : public QWidget
{
	Q_OBJECT

public:
	ControllerBindingWidget_Base(ControllerBindingWidget* parent);
	virtual ~ControllerBindingWidget_Base();

	__fi ControllerSettingsDialog* getDialog() const { return static_cast<ControllerBindingWidget*>(parent())->getDialog(); }
	__fi const std::string& getConfigSection() const { return static_cast<ControllerBindingWidget*>(parent())->getConfigSection(); }
	__fi const std::string& getControllerType() const { return static_cast<ControllerBindingWidget*>(parent())->getControllerType(); }
	__fi u32 getPortNumber() const { return static_cast<ControllerBindingWidget*>(parent())->getPortNumber(); }

protected:
	void initBindingWidgets();
};

class ControllerBindingWidget_DualShock2 final : public ControllerBindingWidget_Base
{
	Q_OBJECT

public:
	ControllerBindingWidget_DualShock2(ControllerBindingWidget* parent);
	~ControllerBindingWidget_DualShock2();

	static ControllerBindingWidget_Base* createInstance(ControllerBindingWidget* parent);

private:
	Ui::ControllerBindingWidget_DualShock2 m_ui;
};

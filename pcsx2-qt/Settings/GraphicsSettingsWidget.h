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

#include "ui_GraphicsSettingsWidget.h"

enum class GSRendererType : s8;

class SettingsDialog;

class GraphicsSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	GraphicsSettingsWidget(SettingsDialog* dialog, QWidget* parent);
	~GraphicsSettingsWidget();

	void updateRendererDependentOptions();

Q_SIGNALS:
	void fullscreenModesChanged(const QStringList& modes);

private Q_SLOTS:
	void onRendererChanged(int index);
	void onAdapterChanged(int index);
	void onEnableHardwareFixesChanged();
	void onIntegerScalingChanged();
	void onTrilinearFilteringChanged();
	void onGpuPaletteConversionChanged(int state);
	void onFullscreenModeChanged(int index);
	void onShadeBoostChanged();

private:
	GSRendererType getEffectiveRenderer() const;

	SettingsDialog* m_dialog;

	Ui::GraphicsSettingsWidget m_ui;

	bool m_hardware_renderer_visible = true;
	bool m_software_renderer_visible = true;
};

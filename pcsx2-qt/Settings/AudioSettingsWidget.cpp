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

#include <QtWidgets/QMessageBox>
#include <algorithm>

#include "AudioSettingsWidget.h"
#include "EmuThread.h"
#include "QtUtils.h"
#include "SettingWidgetBinder.h"
#include "SettingsDialog.h"

static constexpr s32 DEFAULT_INTERPOLATION_MODE = 5;
static constexpr s32 DEFAULT_SYNCHRONIZATION_MODE = 0;
static constexpr s32 DEFAULT_EXPANSION_MODE = 0;
static const char* DEFAULT_OUTPUT_MODULE = "cubeb";
static constexpr s32 DEFAULT_OUTPUT_LATENCY = 100;
static constexpr s32 DEFAULT_VOLUME = 100;
static constexpr s32 DEFAULT_SOUNDTOUCH_SEQUENCE_LENGTH = 30;
static constexpr s32 DEFAULT_SOUNDTOUCH_SEEK_WINDOW = 20;
static constexpr s32 DEFAULT_SOUNDTOUCH_OVERLAP = 10;

static const char* s_output_module_entries[] = {
	QT_TRANSLATE_NOOP("AudioSettingsWidget", "No Sound (Emulate SPU2 only)"),
	QT_TRANSLATE_NOOP("AudioSettingsWidget", "Cubeb (Cross-platform)"),
#ifdef _WIN32
	QT_TRANSLATE_NOOP("AudioSettingsWidget", "XAudio2"),
#endif
	nullptr
};
static const char* s_output_module_values[] = {
	"nullout",
	"cubeb",
#ifdef _WIN32
	"xaudio2",
#endif
	nullptr
};

AudioSettingsWidget::AudioSettingsWidget(SettingsDialog* dialog, QWidget* parent)
	: QWidget(parent)
{
	SettingsInterface* sif = dialog->getSettingsInterface();

	m_ui.setupUi(this);

	SettingWidgetBinder::BindWidgetToIntSetting(sif, m_ui.interpolation, "SPU2/Mixing", "Interpolation", DEFAULT_INTERPOLATION_MODE);
	SettingWidgetBinder::BindWidgetToIntSetting(sif, m_ui.syncMode, "SPU2/Output", "SynchMode", DEFAULT_SYNCHRONIZATION_MODE);
	SettingWidgetBinder::BindWidgetToIntSetting(sif, m_ui.expansionMode, "SPU2/Output", "SpeakerConfiguration", DEFAULT_EXPANSION_MODE);

	SettingWidgetBinder::BindWidgetToEnumSetting(sif, m_ui.outputModule, "SPU2/Output", "OutputModule", s_output_module_entries, s_output_module_values, DEFAULT_OUTPUT_MODULE);
	SettingWidgetBinder::BindWidgetToIntSetting(sif, m_ui.latency, "SPU2/Output", "Latency", DEFAULT_OUTPUT_LATENCY);
	connect(m_ui.latency, &QSlider::valueChanged, this, &AudioSettingsWidget::updateLatencyLabel);

	SettingWidgetBinder::BindWidgetToIntSetting(sif, m_ui.volume, "SPU2/Mixing", "FinalVolume", DEFAULT_VOLUME);
	connect(m_ui.volume, &QSlider::valueChanged, this, &AudioSettingsWidget::updateVolumeLabel);

	SettingWidgetBinder::BindWidgetToIntSetting(sif, m_ui.sequenceLength, "Soundtouch", "SequenceLengthMS", DEFAULT_SOUNDTOUCH_SEQUENCE_LENGTH);
	connect(m_ui.sequenceLength, &QSlider::valueChanged, this, &AudioSettingsWidget::updateTimestretchSequenceLengthLabel);
	SettingWidgetBinder::BindWidgetToIntSetting(sif, m_ui.seekWindowSize, "Soundtouch", "SeekWindowMS", DEFAULT_SOUNDTOUCH_SEEK_WINDOW);
	connect(m_ui.seekWindowSize, &QSlider::valueChanged, this, &AudioSettingsWidget::updateTimestretchSeekwindowLengthLabel);
	SettingWidgetBinder::BindWidgetToIntSetting(sif, m_ui.overlap, "Soundtouch", "OverlapMS", DEFAULT_SOUNDTOUCH_OVERLAP);
	connect(m_ui.overlap, &QSlider::valueChanged, this, &AudioSettingsWidget::updateTimestretchOverlapLabel);

	updateVolumeLabel();
	updateLatencyLabel();
	updateTimestretchSequenceLengthLabel();
	updateTimestretchSeekwindowLengthLabel();
	updateTimestretchOverlapLabel();
}

AudioSettingsWidget::~AudioSettingsWidget() = default;

void AudioSettingsWidget::updateVolumeLabel()
{
	m_ui.volumeLabel->setText(tr("%1%").arg(m_ui.volume->value()));
}

void AudioSettingsWidget::updateLatencyLabel()
{
	m_ui.latencyLabel->setText(tr("%1 ms (avg)").arg(m_ui.latency->value()));
}

void AudioSettingsWidget::updateTimestretchSequenceLengthLabel()
{
	m_ui.sequenceLengthLabel->setText(tr("%1 ms").arg(m_ui.sequenceLength->value()));
}

void AudioSettingsWidget::updateTimestretchSeekwindowLengthLabel()
{
	m_ui.seekWindowSizeLabel->setText(tr("%1 ms").arg(m_ui.seekWindowSize->value()));
}

void AudioSettingsWidget::updateTimestretchOverlapLabel()
{
	m_ui.overlapLabel->setText(tr("%1 ms").arg(m_ui.overlap->value()));
}

void AudioSettingsWidget::resetTimestretchDefaults()
{
	m_ui.sequenceLength->setValue(DEFAULT_SOUNDTOUCH_SEQUENCE_LENGTH);
	m_ui.seekWindowSize->setValue(DEFAULT_SOUNDTOUCH_SEEK_WINDOW);
	m_ui.overlap->setValue(DEFAULT_SOUNDTOUCH_OVERLAP);
}

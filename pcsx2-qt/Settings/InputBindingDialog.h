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
#include "ui_InputBindingDialog.h"
#include "Frontend/InputManager.h"
#include <QtWidgets/QDialog>
#include <optional>
#include <string>
#include <vector>

class InputBindingDialog : public QDialog
{
	Q_OBJECT

public:
	InputBindingDialog(std::string section_name, std::string key_name, std::vector<std::string> bindings, QWidget* parent);
	~InputBindingDialog();

protected Q_SLOTS:
	void onAddBindingButtonClicked();
	void onRemoveBindingButtonClicked();
	void onClearBindingsButtonClicked();
	void onInputListenTimerTimeout();
	void inputManagerHookCallback(InputBindingKey key, float value);

protected:
	enum : u32
	{
		TIMEOUT_FOR_BINDING = 5
	};

	virtual bool eventFilter(QObject* watched, QEvent* event) override;

	virtual void startListeningForInput(u32 timeout_in_seconds);
	virtual void stopListeningForInput();

	bool isListeningForInput() const { return m_input_listen_timer != nullptr; }
	void addNewBinding();

	void updateList();
	void saveListToSettings();

	void hookInputManager();
	void unhookInputManager();

	Ui::InputBindingDialog m_ui;

	std::string m_section_name;
	std::string m_key_name;
	std::vector<std::string> m_bindings;
	std::vector<InputBindingKey> m_new_bindings;

	QTimer* m_input_listen_timer = nullptr;
	u32 m_input_listen_remaining_seconds = 0;
};

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

#include "common/FileSystem.h"
#include "common/StringUtil.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "Settings/CreateMemoryCardDialog.h"

#include "pcsx2/MemoryCardFile.h"
#include "pcsx2/System.h"

CreateMemoryCardDialog::CreateMemoryCardDialog(QWidget* parent /* = nullptr */)
	: QDialog(parent)
{
	m_ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	connect(m_ui.name, &QLineEdit::textChanged, this, &CreateMemoryCardDialog::nameTextChanged);

	connect(m_ui.size8MB, &QRadioButton::clicked, this, [this]() { setType(MemoryCardType::File, MemoryCardFileType::PS2_8MB); });
	connect(m_ui.size16MB, &QRadioButton::clicked, this, [this]() { setType(MemoryCardType::File, MemoryCardFileType::PS2_16MB); });
	connect(m_ui.size32MB, &QRadioButton::clicked, this, [this]() { setType(MemoryCardType::File, MemoryCardFileType::PS2_32MB); });
	connect(m_ui.size64MB, &QRadioButton::clicked, this, [this]() { setType(MemoryCardType::File, MemoryCardFileType::PS2_64MB); });
	connect(m_ui.size128KB, &QRadioButton::clicked, this, [this]() { setType(MemoryCardType::File, MemoryCardFileType::PS1); });
	connect(m_ui.sizeFolder, &QRadioButton::clicked, this, [this]() { setType(MemoryCardType::Folder, MemoryCardFileType::Unknown); });

	disconnect(m_ui.buttonBox, &QDialogButtonBox::accepted, this, nullptr);

	connect(m_ui.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &CreateMemoryCardDialog::createCard);
	connect(m_ui.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &CreateMemoryCardDialog::close);
	connect(m_ui.buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &CreateMemoryCardDialog::restoreDefaults);

#ifdef _WIN32
	m_ui.ntfsCompression->setEnabled(false);
#endif

	updateState();
}

CreateMemoryCardDialog::~CreateMemoryCardDialog() = default;

void CreateMemoryCardDialog::nameTextChanged()
{
	const QString controlName(m_ui.name->text());
	const int cursorPos = m_ui.name->cursorPosition();

	QString nameWithoutExtension;
	if (controlName.endsWith(QStringLiteral(".ps2")))
		nameWithoutExtension = controlName.left(controlName.size() - 4);
	else
		nameWithoutExtension = controlName;

	QSignalBlocker sb(m_ui.name);
	if (nameWithoutExtension.isEmpty())
		m_ui.name->setText(QString());
	else
		m_ui.name->setText(nameWithoutExtension + QStringLiteral(".ps2"));

	m_ui.name->setCursorPosition(cursorPos);
	updateState();
}

void CreateMemoryCardDialog::setType(MemoryCardType type, MemoryCardFileType fileType)
{
	m_type = type;
	m_fileType = fileType;
	updateState();
}

void CreateMemoryCardDialog::restoreDefaults()
{
	setType(MemoryCardType::File, MemoryCardFileType::PS2_8MB);
	m_ui.size8MB->setChecked(true);
	m_ui.size16MB->setChecked(false);
	m_ui.size32MB->setChecked(false);
	m_ui.size64MB->setChecked(false);
	m_ui.size128KB->setChecked(false);
	m_ui.sizeFolder->setChecked(false);
}

void CreateMemoryCardDialog::updateState()
{
	const bool okay = (m_ui.name->text().length() > 4);

	m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(okay);
#ifdef _WIN32
	m_ui.ntfsCompression->setEnabled(m_type == MemoryCardType::File);
#endif
}

void CreateMemoryCardDialog::createCard()
{
	const QString name(m_ui.name->text());
	const std::string nameStr(name.toStdString());
	if (FileMcd_GetCardInfo(nameStr).has_value())
	{
		QMessageBox::critical(this, tr("Create Memory Card"),
			tr("Failed to create the memory card, because another card with the name '%1' already exists.").arg(name));
		return;
	}

	if (!FileMcd_CreateNewCard(nameStr, m_type, m_fileType))
	{
		QMessageBox::critical(this, tr("Create Memory Card"),
			tr("Failed to create the memory card, the log may contain more information."));
		return;
	}

#ifdef  _WIN32
	if (m_ui.ntfsCompression->isChecked() && m_type == MemoryCardType::File)
	{
		const std::string fullPath(Path::CombineStdString(EmuFolders::MemoryCards, nameStr));
		FileSystem::SetPathCompression(fullPath.c_str(), true);
	}
#endif

	QMessageBox::information(this, tr("Create Memory Card"), tr("Memory card '%1' created.").arg(name));
	accept();
}

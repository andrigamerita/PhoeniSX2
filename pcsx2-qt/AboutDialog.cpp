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

#include "AboutDialog.h"
#include "QtUtils.h"
#include "svnrev.h"
#include <QtCore/QString>
#include <QtWidgets/QDialog>

AboutDialog::AboutDialog(QWidget* parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setFixedSize(geometry().width(), geometry().height());

	m_ui.scmversion->setTextInteractionFlags(Qt::TextSelectableByMouse);
	m_ui.scmversion->setText(GIT_REV);

	m_ui.links->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_ui.links->setOpenExternalLinks(true);
	m_ui.links->setText(QStringLiteral(
		R"(<a href="%1">%2</a> | <a href="%3">%4</a> | <a href="%5">%6</a></a>)")
							.arg(getWebsiteUrl())
							.arg(tr("Website"))
							.arg(getDiscordServerUrl())
							.arg(tr("Discord Server"))
							.arg(getPatreonUrl())
							.arg(tr("News and Updates (Patreon)")));

	connect(m_ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);
}

AboutDialog::~AboutDialog() = default;

QString AboutDialog::getWebsiteUrl()
{
	return QStringLiteral("https://www.aethersx2.com/");
}

QString AboutDialog::getDiscordServerUrl()
{
	return QStringLiteral("https://discord.com/invite/JZ7BkeEdrJ");
}

QString AboutDialog::getPatreonUrl()
{
	return QStringLiteral("https://www.patreon.com/aethersx2");
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "startupfirstpage.h"
#include "xmlsettingsmanager.h"
#include "sessionsettingsmanager.h"

namespace LC::BitTorrent
{
	StartupFirstPage::StartupFirstPage (SessionSettingsManager *ssm, QWidget *parent)
	: QWizardPage { parent }
	, SSM_ { ssm }
	{
		Ui_.setupUi (this);

		setTitle (QStringLiteral ("BitTorrent"));
		setSubTitle (tr ("Set basic options"));

		setProperty ("WizardType", 1);
	}

	void StartupFirstPage::initializePage ()
	{
		connect (wizard (),
				&QWizard::accepted,
				this,
				[this]
				{
					const QList<QVariant> ports
					{
						Ui_.LowerPort_->value (),
						Ui_.UpperPort_->value ()
					};
					auto& xsm = XmlSettingsManager::Instance ();
					xsm.setProperty ("TCPPortRange", ports);
					xsm.setProperty ("MaxUploads", Ui_.UploadConnections_->value ());
					xsm.setProperty ("MaxConnections", Ui_.TotalConnections_->value ());

					const auto idx = Ui_.SettingsSet_->currentIndex ();
					const auto preset = static_cast<SessionSettingsManager::Preset> (idx);
					SSM_->SetPreset (preset);
				});
	}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "startupfirstpage.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "sessionsettingsmanager.h"

namespace LC
{
namespace BitTorrent
{
	StartupFirstPage::StartupFirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		setTitle ("BitTorrent");
		setSubTitle (tr ("Set basic options"));

		setProperty ("WizardType", 1);
	}

	void StartupFirstPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));
	}

	void StartupFirstPage::handleAccepted ()
	{
		const QList<QVariant> ports
		{
			Ui_.LowerPort_->value (),
			Ui_.UpperPort_->value ()
		};
		XmlSettingsManager::Instance ()->setProperty ("TCPPortRange", ports);

		XmlSettingsManager::Instance ()->setProperty ("MaxUploads",
				Ui_.UploadConnections_->value ());
		XmlSettingsManager::Instance ()->setProperty ("MaxConnections",
				Ui_.TotalConnections_->value ());

		const auto idx = Ui_.SettingsSet_->currentIndex ();
		const auto sset = static_cast<SessionSettingsManager::Preset> (idx);
		Core::Instance ()->GetSessionSettingsManager ()->SetPreset (sset);
	}
}
}

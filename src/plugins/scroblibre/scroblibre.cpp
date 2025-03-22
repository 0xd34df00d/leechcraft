/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "scroblibre.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/sll/unreachable.h>
#include "xmlsettingsmanager.h"
#include "accountsmanager.h"
#include "authmanager.h"

namespace LC
{
namespace Scroblibre
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		AccMgr_ = new AccountsManager (this);
		AuthMgr_ = new AuthManager (proxy, this);
		connect (AccMgr_,
				SIGNAL (accountAdded (QUrl, QString)),
				AuthMgr_,
				SLOT (handleAccountAdded (QUrl, QString)));
		connect (AccMgr_,
				SIGNAL (accountRemoved (QUrl, QString)),
				AuthMgr_,
				SLOT (handleAccountRemoved (QUrl, QString)));

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "scroblibresettings.xml");
		XSD_->SetDataSource ("AccountsView", AccMgr_->GetModel ());
	}

	void Plugin::SecondInit ()
	{
		AccMgr_->LoadAccounts ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Scroblibre";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Scroblibre";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Multiaccount scrobbler for services supporting Scrobbler API 1.2.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	bool Plugin::SupportsFeature (Feature feature) const
	{
		switch (feature)
		{
		case Feature::Backdating:
			return false;
		}

		Util::Unreachable ();
	}

	QString Plugin::GetServiceName () const
	{
		return "Scrobbler API 1.2";
	}

	void Plugin::NowPlaying (const Media::AudioInfo& info)
	{
		AuthMgr_->HandleAudio (info);
	}

	void Plugin::SendBackdated (const BackdatedTracks_t&)
	{
	}

	void Plugin::PlaybackStopped ()
	{
		AuthMgr_->HandleStopped ();
	}

	void Plugin::LoveCurrentTrack ()
	{
	}

	void Plugin::BanCurrentTrack ()
	{
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_scroblibre, LC::Scroblibre::Plugin);

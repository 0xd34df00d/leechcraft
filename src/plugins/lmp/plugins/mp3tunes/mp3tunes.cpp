/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mp3tunes.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include "xmlsettingsmanager.h"
#include "accountsmanager.h"
#include "authmanager.h"
#include "uploader.h"
#include "playlistmanager.h"

namespace LC
{
namespace LMP
{
namespace MP3Tunes
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		auto nam = proxy->GetNetworkAccessManager ();
		AuthMgr_ = new AuthManager (nam, proxy, this);

		AccMgr_ = new AccountsManager ();

		PLManager_ = new PlaylistManager (nam, AuthMgr_, AccMgr_, this);

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lmpmp3tunessettings.xml");
		XSD_->SetDataSource ("AccountsView", AccMgr_->GetAccModel ());

		connect (AccMgr_,
				SIGNAL (accountsChanged ()),
				this,
				SIGNAL (accountsChanged ()));
	}

	void Plugin::SecondInit ()
	{
		PLManager_->Update ();
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.MP3Tunes";
	}

	QString Plugin::GetName () const
	{
		return "LMP MP3tunes";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for the MP3tunes.com service, including its playlist and locker facilities.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.CloudStorage";
		result << "org.LeechCraft.LMP.PlaylistProvider";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr)
	{
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QString Plugin::GetCloudName () const
	{
		return "MP3tunes";
	}

	QIcon Plugin::GetCloudIcon () const
	{
		return QIcon ();
	}

	QStringList Plugin::GetSupportedFileFormats () const
	{
		return { "m4a", "mp3", "mp4", "ogg" };
	}

	void Plugin::Upload (const QString& acc, const QString& localPath)
	{
		if (!Uploaders_.contains (acc))
		{
			auto up = new Uploader (acc,
				Proxy_->GetNetworkAccessManager (),
				AuthMgr_,
				this);
			Uploaders_ [acc] = up;

			connect (up,
					SIGNAL (uploadFinished (QString, LC::LMP::CloudStorageError, QString)),
					this,
					SIGNAL (uploadFinished (QString, LC::LMP::CloudStorageError, QString)));
		}

		Uploaders_ [acc]->Upload (localPath);
	}

	QStringList Plugin::GetAccounts () const
	{
		return AccMgr_->GetAccounts ();
	}

	QStandardItem* Plugin::GetPlaylistsRoot () const
	{
		return PLManager_->GetRoot ();
	}

	void Plugin::UpdatePlaylists ()
	{
		PLManager_->Update ();
	}

	boost::optional<Media::AudioInfo> Plugin::GetURLInfo (const QUrl& url)
	{
		return PLManager_->GetMediaInfo (url);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_mp3tunes, LC::LMP::MP3Tunes::Plugin);

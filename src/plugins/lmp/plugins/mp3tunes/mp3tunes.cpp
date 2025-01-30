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
#include <util/threads/coro.h>
#include <interfaces/core/icoreproxy.h>
#include "xmlsettingsmanager.h"
#include "accountsmanager.h"
#include "authmanager.h"
#include "uploader.h"
#include "playlistmanager.h"

namespace LC::LMP::MP3Tunes
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		AuthMgr_ = new AuthManager { this };
		AccMgr_ = new AccountsManager {};
		PLManager_ = new PlaylistManager (AuthMgr_, AccMgr_, this);

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
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
		return
		{
			"org.LeechCraft.LMP.CloudStorage",
			"org.LeechCraft.LMP.PlaylistProvider",
		};
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

	Util::ContextTask<Plugin::UploadResult> Plugin::Upload (const QString& acc, const QString& localPath)
	{
		if (!Uploaders_.contains (acc))
			Uploaders_ [acc] = new Uploader (acc, AuthMgr_, this);

		return Uploaders_ [acc]->Upload (localPath);
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

	std::optional<Media::AudioInfo> Plugin::GetURLInfo (const QUrl& url)
	{
		return PLManager_->GetMediaInfo (url);
	}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_mp3tunes, LC::LMP::MP3Tunes::Plugin);

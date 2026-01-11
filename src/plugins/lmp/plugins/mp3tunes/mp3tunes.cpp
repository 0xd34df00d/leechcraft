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
#include <util/sll/qtutil.h>
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
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lmpmp3tunessettings.xml"_qs);
		XSD_->SetDataSource ("AccountsView"_qs, AccMgr_->GetAccModel ());
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
		return "org.LeechCraft.LMP.MP3Tunes"_qba;
	}

	QString Plugin::GetName () const
	{
		return "LMP MP3tunes"_qs;
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
			"org.LeechCraft.LMP.CloudStorage"_qba,
			"org.LeechCraft.LMP.PlaylistProvider"_qba,
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

	QString Plugin::GetSyncSystemName () const
	{
		return "MP3tunes"_qs;
	}

	QAbstractItemModel& Plugin::GetSyncTargetsModel ()
	{
		return *AccMgr_->GetAccModel ();
	}

	void Plugin::RefreshSyncTargets ()
	{
	}

	ISyncPluginConfigWidget_ptr Plugin::MakeConfigWidget (const QModelIndex&)
	{
		return {};
	}

	Util::ContextTask<Plugin::UploadResult> Plugin::Upload (UploadJob job)
	{
		const auto& acc = job.Target_.data ().toString ();

		auto& uploader = Uploaders_ [acc];
		if (!uploader)
			uploader = new Uploader (acc, AuthMgr_, this);
		return uploader->Upload (job.LocalPath_);
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

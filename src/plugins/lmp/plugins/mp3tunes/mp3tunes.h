/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/iplaylistprovider.h>
#include <interfaces/lmp/isyncplugin.h>

namespace LC::LMP::MP3Tunes
{
	class AccountsManager;
	class AuthManager;
	class Uploader;
	class PlaylistManager;

	class Plugin
		: public QObject
		, public IInfo
		, public IHaveSettings
		, public IPlugin2
		, public ILMPPlugin
		, public ISyncPlugin
		, public IPlaylistProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
			IHaveSettings
			IPlugin2
			LC::LMP::ILMPPlugin
			LC::LMP::ISyncPlugin
			LC::LMP::IPlaylistProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.MP3Tunes")

		AccountsManager *AccMgr_;
		Util::XmlSettingsDialog_ptr XSD_;

		AuthManager *AuthMgr_;

		PlaylistManager *PLManager_;

		QMap<QString, Uploader*> Uploaders_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;

		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		void SetLMPProxy (ILMPProxy_ptr) override;

		QObject* GetQObject () override;
		QString GetSyncSystemName () const override;
		QAbstractItemModel& GetSyncTargetsModel () override;
		void RefreshSyncTargets () override;
		ISyncPluginConfigWidget_ptr MakeConfigWidget (const QModelIndex&) override;
		Util::ContextTask<UploadResult> Upload (UploadJob uploadJob) override;

		QStandardItem* GetPlaylistsRoot () const override;
		void UpdatePlaylists () override;
		std::optional<Media::AudioInfo> GetURLInfo (const QUrl&) override;
	};
}

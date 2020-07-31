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
#include <interfaces/lmp/icloudstorageplugin.h>
#include <interfaces/lmp/iplaylistprovider.h>

namespace LC
{
namespace LMP
{
namespace MP3Tunes
{
	class AccountsManager;
	class AuthManager;
	class Uploader;
	class PlaylistManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IPlugin2
				 , public ILMPPlugin
				 , public ICloudStoragePlugin
				 , public IPlaylistProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveSettings
				IPlugin2
				LC::LMP::ILMPPlugin
				LC::LMP::ICloudStoragePlugin
				LC::LMP::IPlaylistProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.MP3Tunes")

		ICoreProxy_ptr Proxy_;

		AccountsManager *AccMgr_;
		Util::XmlSettingsDialog_ptr XSD_;

		AuthManager *AuthMgr_;

		PlaylistManager *PLManager_;

		QMap<QString, Uploader*> Uploaders_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;

		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		void SetLMPProxy (ILMPProxy_ptr);

		QObject* GetQObject ();
		QString GetCloudName () const;
		QIcon GetCloudIcon () const;
		QStringList GetSupportedFileFormats () const;
		void Upload (const QString& account, const QString& filename);
		QStringList GetAccounts () const;

		QStandardItem* GetPlaylistsRoot () const;
		void UpdatePlaylists ();
		boost::optional<Media::AudioInfo> GetURLInfo (const QUrl&);
	signals:
		void uploadFinished (const QString&,
				LC::LMP::CloudStorageError, const QString&);
		void accountsChanged ();
	};
}
}
}

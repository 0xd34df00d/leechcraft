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
#include <interfaces/media/iaudiopile.h>
#include <interfaces/media/iradiostationprovider.h>
#include <interfaces/media/irestorableradiostationprovider.h>

class QStandardItemModel;

namespace LC
{
namespace Util
{
	class QueueManager;

	namespace SvcAuth
	{
		class VkAuthManager;
	}
}

namespace TouchStreams
{
	class AlbumsManager;
	class FriendsManager;
	class RecsManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public Media::IAudioPile
				 , public Media::IRadioStationProvider
				 , public Media::IRestorableRadioStationProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveSettings
				Media::IAudioPile
				Media::IRadioStationProvider
				Media::IRestorableRadioStationProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.TouchStreams")

		ICoreProxy_ptr Proxy_;
		Util::QueueManager *Queue_;

		Util::XmlSettingsDialog_ptr XSD_;
		Util::SvcAuth::VkAuthManager *AuthMgr_;

		QStandardItemModel *Model_;

		AlbumsManager *AlbumsMgr_;
		FriendsManager *FriendsMgr_;
		RecsManager *RecsManager_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QString GetServiceName () const;
		QIcon GetServiceIcon () const;
		QFuture<Media::IAudioPile::Result_t> Search (const Media::AudioSearchRequest&);

		QList<QAbstractItemModel*> GetRadioListItems () const;
		Media::IRadioStation_ptr GetRadioStation (const QModelIndex&, const QString&);
		void RefreshItems (const QList<QModelIndex>&);

		QFuture<Media::RadiosRestoreResult_t> RestoreRadioStations (const QStringList&);
	private slots:
		void saveCookies (const QByteArray&);
	};
}
}

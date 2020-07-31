/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumsmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardItem>
#include <QIcon>
#include <QTimer>
#include <QtDebug>
#include <util/svcauth/vkauthmanager.h>
#include <util/sll/queuemanager.h>
#include <util/sll/parsejson.h>
#include <util/sll/urloperator.h>
#include <util/sll/qtutil.h>
#include <interfaces/media/iradiostationprovider.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/core/iiconthememanager.h>
#include "util.h"

namespace LC
{
namespace TouchStreams
{
	namespace
	{
		enum Role
		{
			AlbumID = Media::RadioItemRole::MaxRadioRole + 1
		};
	}

	AlbumsManager::AlbumInfo::AlbumInfo (qlonglong id, const QString& name, QStandardItem *item)
	: ID_ { id }
	, Name_ { name }
	, Item_ { item }
	{
	}

	AlbumsManager::AlbumsManager (Util::SvcAuth::VkAuthManager *authMgr,
			ICoreProxy_ptr proxy, QObject *parent)
	: AlbumsManager (-1, authMgr, proxy, parent)
	{
	}

	AlbumsManager::AlbumsManager (qlonglong id, Util::SvcAuth::VkAuthManager *authMgr,
			ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, UserID_ (id)
	, AuthMgr_ (authMgr)
	, RequestQueueGuard_ (AuthMgr_->ManageQueue (&RequestQueue_))
	, AlbumsRootItem_ (new QStandardItem (tr ("VKontakte: your audio")))
	{
		InitRootItem ();

		QTimer::singleShot (1000,
				this,
				SLOT (refetchAlbums ()));

		connect (AuthMgr_,
				SIGNAL (justAuthenticated ()),
				this,
				SLOT (refetchAlbums ()));
	}

	AlbumsManager::AlbumsManager (qlonglong id, const QVariant& albums, const QVariant& tracks,
			Util::SvcAuth::VkAuthManager *authMgr, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, UserID_ (id)
	, AuthMgr_ (authMgr)
	, RequestQueueGuard_ (AuthMgr_->ManageQueue (&RequestQueue_))
	, AlbumsRootItem_ (new QStandardItem (tr ("VKontakte: your audio")))
	{
		InitRootItem ();

		HandleAlbums (albums);
		HandleTracks (tracks);
	}

	QStandardItem* AlbumsManager::GetRootItem () const
	{
		return AlbumsRootItem_;
	}

	qlonglong AlbumsManager::GetUserID () const
	{
		return UserID_;
	}

	quint32 AlbumsManager::GetTracksCount () const
	{
		return TracksCount_;
	}

	void AlbumsManager::RefreshItems (QList<QStandardItem*>& items)
	{
		for (const auto item : items)
		{
			auto parent = item;
			while (parent)
			{
				if (AlbumsRootItem_ != parent)
				{
					parent = parent->parent ();
					continue;
				}

				if (auto rc = AlbumsRootItem_->rowCount ())
					AlbumsRootItem_->removeRows (0, rc);
				Albums_.clear ();
				refetchAlbums ();

				items.removeOne (item);
				return;
			}
		}
	}

	void AlbumsManager::InitRootItem ()
	{
		static QIcon vkIcon { ":/touchstreams/resources/images/vk.svg" };
		AlbumsRootItem_->setIcon (vkIcon);
		AlbumsRootItem_->setEditable (false);
		AlbumsRootItem_->setData (Media::RadioType::TracksRoot, Media::RadioItemRole::ItemType);
	}

	bool AlbumsManager::HandleAlbums (const QVariant& albumsListVar)
	{
		auto albumsList = albumsListVar.toList ();

		if (albumsList.isEmpty ())
		{
			emit finished (this);
			return false;
		}

		albumsList.removeFirst ();

		if (auto rc = AlbumsRootItem_->rowCount ())
			AlbumsRootItem_->removeRows (0, rc);
		Albums_.clear ();

		const auto& icon = Proxy_->GetIconThemeManager ()->GetIcon ("media-optical");

		auto allItem = new QStandardItem (tr ("Uncategorized"));
		allItem->setEditable (false);
		allItem->setData (-1, Role::AlbumID);
		allItem->setData (Media::RadioType::TracksRoot, Media::RadioItemRole::ItemType);
		allItem->setIcon (icon);
		AlbumsRootItem_->appendRow (allItem);
		Albums_ [-1] = AlbumInfo { -1, allItem->text (), allItem };

		for (const auto& albumMap : albumsList)
		{
			const auto& map = albumMap.toMap ();

			const auto id = map ["album_id"].toLongLong ();
			const auto& name = map ["title"].toString ();

			auto item = new QStandardItem (name);
			item->setEditable (false);
			item->setIcon (icon);
			item->setData (Media::RadioType::TracksRoot, Media::RadioItemRole::ItemType);
			item->setData (id, Role::AlbumID);
			Albums_ [id] = AlbumInfo { id, name, item };

			AlbumsRootItem_->appendRow (item);
		}

		return true;
	}

	bool AlbumsManager::HandleTracks (const QVariant& tracksListVar)
	{
		for (const auto& trackVar : tracksListVar.toMap () ["items"].toList ())
		{
			const auto& map = trackVar.toMap ();

			const auto albumId = map.value ("album_id", "-1").toLongLong ();
			auto albumItem = Albums_ [albumId].Item_;
			if (!albumItem)
			{
				qWarning () << Q_FUNC_INFO
						<< "no album item for album"
						<< albumId;
				continue;
			}

			const auto& maybeInfo = TrackMap2Info (map);
			if (!maybeInfo)
				continue;

			const auto& info = *maybeInfo;

			auto trackItem = new QStandardItem (QString::fromUtf8 ("%1 — %2")
						.arg (info.Artist_)
						.arg (info.Title_));
			trackItem->setEditable (false);
			trackItem->setData (Media::RadioType::SingleTrack, Media::RadioItemRole::ItemType);
			trackItem->setData (TrackMap2RadioId (map), Media::RadioItemRole::RadioID);
			trackItem->setData ("org.LeechCraft.TouchStreams", Media::RadioItemRole::PluginID);
			trackItem->setData (QVariant::fromValue<QList<Media::AudioInfo>> ({ info }),
					Media::RadioItemRole::TracksInfos);
			albumItem->appendRow (trackItem);

			++TracksCount_;
		}

		return true;
	}

	void AlbumsManager::refetchAlbums ()
	{
		if (!CheckAuthentication (AlbumsRootItem_, AuthMgr_, Proxy_))
			return;

		RequestQueue_.append ({
				[this] (const QString& key) -> void
				{
					QUrl url ("https://api.vk.com/method/audio.getAlbums");
					Util::UrlOperator { url }
							("access_token", key)
							("count", "100")
							(UserID_ >= 0, "uid", UserID_);

					auto nam = Proxy_->GetNetworkAccessManager ();
					connect (nam->get (QNetworkRequest (url)),
							SIGNAL (finished ()),
							this,
							SLOT (handleAlbumsFetched ()));
				},
				Util::QueuePriority::Normal
			});
		AuthMgr_->GetAuthKey ();
	}

	void AlbumsManager::handleAlbumsFetched ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();
		HandleAlbums (data ["response"]);

		RequestQueue_.prepend ({
				[this] (const QString& key)
				{
					QUrl url ("https://api.vk.com/method/audio.get");
					Util::UrlOperator { url }
							("v", "5.37")
							("access_token", key)
							("count", "6000")
							(UserID_ >= 0, "owner_id", UserID_);

					auto nam = Proxy_->GetNetworkAccessManager ();
					connect (nam->get (QNetworkRequest (url)),
							SIGNAL (finished ()),
							this,
							SLOT (handleTracksFetched ()));
				},
				Util::QueuePriority::High
			});
		AuthMgr_->GetAuthKey ();
	}

	void AlbumsManager::handleTracksFetched ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();
		HandleTracks (data ["response"]);

		emit finished (this);
	}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "albumsmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardItem>
#include <QTimer>
#include <QtDebug>
#include <qjson/parser.h>
#include <util/svcauth/vkauthmanager.h>
#include <util/queuemanager.h>
#include <interfaces/media/iradiostationprovider.h>
#include <interfaces/media/audiostructs.h>

namespace LeechCraft
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

	AlbumsManager::AlbumsManager (Util::SvcAuth::VkAuthManager *authMgr,
			Util::QueueManager *queue, ICoreProxy_ptr proxy, QObject *parent)
	: AlbumsManager (-1, authMgr, queue, proxy, parent)
	{
	}

	AlbumsManager::AlbumsManager (qlonglong id, Util::SvcAuth::VkAuthManager *authMgr,
			Util::QueueManager *queue, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, UserID_ (id)
	, AuthMgr_ (authMgr)
	, Queue_ (queue)
	, AlbumsRootItem_ (new QStandardItem (tr ("VKontakte: your audio")))
	{
		AlbumsRootItem_->setEditable (false);

		QTimer::singleShot (1000,
				this,
				SLOT (refetchAlbums ()));

		AuthMgr_->ManageQueue (&RequestQueue_);
	}

	AlbumsManager::~AlbumsManager ()
	{
		AuthMgr_->UnmanageQueue (&RequestQueue_);
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

	void AlbumsManager::refetchAlbums ()
	{
		RequestQueue_.append ({
				[this] (const QString& key) -> void
				{
					QUrl url ("https://api.vk.com/method/audio.getAlbums");
					url.addQueryItem ("access_token", key);
					url.addQueryItem ("count", "100");
					if (UserID_ >= 0)
						url.addQueryItem ("uid", QString::number (UserID_));

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

		const auto& data = QJson::Parser ().parse (reply).toMap ();
		auto albumsList = data ["response"].toList ();
		if (albumsList.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty albums list"
					<< data;
			emit finished (this);
			return;
		}

		albumsList.removeFirst ();

		if (auto rc = AlbumsRootItem_->rowCount ())
			AlbumsRootItem_->removeRows (0, rc);
		Albums_.clear ();

		const auto& icon = Proxy_->GetIcon ("media-optical");

		auto allItem = new QStandardItem (tr ("Uncategorized"));
		allItem->setEditable (false);
		allItem->setData (-1, Role::AlbumID);
		allItem->setData (Media::RadioType::TracksList, Media::RadioItemRole::ItemType);
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
			item->setData (Media::RadioType::TracksList, Media::RadioItemRole::ItemType);
			item->setData (id, Role::AlbumID);
			Albums_ [id] = AlbumInfo { id, name, item };

			AlbumsRootItem_->appendRow (item);
		}

		RequestQueue_.prepend ({
				[this] (const QString& key) -> void
				{
					QUrl url ("https://api.vk.com/method/audio.get");
					url.addQueryItem ("access_token", key);
					url.addQueryItem ("count", "1000");
					if (UserID_ >= 0)
						url.addQueryItem ("uid", QString::number (UserID_));

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

		const auto& data = QJson::Parser ().parse (reply).toMap ();
		auto tracksList = data ["response"].toList ();

		QHash<qlonglong, QList<Media::AudioInfo>> album2urls;
		for (const auto& trackVar : tracksList)
		{
			const auto& map = trackVar.toMap ();

			const auto albumId = map.value ("album", "-1").toLongLong ();
			const auto& url = QUrl::fromEncoded (map ["url"].toString ().toUtf8 ());
			if (!url.isValid ())
				continue;

			Media::AudioInfo info {};
			info.Title_ = map ["title"].toString ();
			info.Artist_ = map ["artist"].toString ();
			info.Length_ = map ["duration"].toInt ();
			info.Other_ ["URL"] = url;

			album2urls [albumId] << info;

			auto albumItem = Albums_ [albumId].Item_;

			auto trackItem = new QStandardItem (QString::fromUtf8 ("%1 — %2")
						.arg (info.Artist_)
						.arg (info.Title_));
			trackItem->setEditable (false);
			trackItem->setData (Media::RadioType::SingleTrack, Media::RadioItemRole::ItemType);
			trackItem->setData (QVariant::fromValue<QList<Media::AudioInfo>> ({ info }),
					Media::RadioItemRole::TracksInfos);
			albumItem->appendRow (trackItem);

			++TracksCount_;
		}

		for (auto i = album2urls.begin (); i != album2urls.end (); ++i)
		{
			auto item = Albums_ [i.key ()].Item_;
			item->setData (QVariant::fromValue (i.value ()), Media::RadioItemRole::TracksInfos);
		}

		emit finished (this);
	}
}
}

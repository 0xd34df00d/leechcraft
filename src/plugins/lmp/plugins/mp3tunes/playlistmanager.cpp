/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlistmanager.h"
#include <QStandardItem>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/threads/coro/asdomdocument.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/lmp/iplaylistprovider.h>
#include "authmanager.h"
#include "accountsmanager.h"
#include "consts.h"

namespace LC::LMP::MP3Tunes
{
	PlaylistManager::PlaylistManager (AuthManager *authMgr, AccountsManager *accMgr, QObject *parent)
	: QObject { parent }
	, AuthMgr_ { authMgr }
	, AccMgr_ { accMgr }
	, Root_ { new QStandardItem { "mp3tunes.com"_qs } }
	{
		Root_->setEditable (false);
		connect (AccMgr_,
				&AccountsManager::accountsChanged,
				this,
				&PlaylistManager::Update,
				Qt::QueuedConnection);
	}

	QStandardItem* PlaylistManager::GetRoot () const
	{
		return Root_;
	}

	struct PlaylistTrack
	{
		QUrl Url_;
		Media::AudioInfo Info_;
	};

	struct Playlist
	{
		QString Id_;
		QString Title_;
		QList<PlaylistTrack> Tracks_;
	};

	using PlaylistTracksFetchResult = Util::Either<QString, QList<PlaylistTrack>>;

	namespace
	{
		Util::Task<PlaylistTracksFetchResult> FetchTracks (const QString& playlistId, const QString& sid)
		{
			const auto& url = "https://ws.mp3tunes.com/api/v1/lockerData?output=xml&sid=%1&partner_token=%2&type=track&playlist_id=%3"_qs
					.arg (sid, Consts::PartnerId, playlistId);
			const auto response = co_await *GetProxyHolder ()->GetNetworkAccessManager ()->get (QNetworkRequest { url });
			const auto data = co_await response.ToEither ();
			const auto doc = co_await Util::AsDomDocument { data, PlaylistManager::tr ("unable to parse server response") };

			QVector<PlaylistTrack> tracks;
			const auto& trackList = doc.documentElement ().firstChildElement ("trackList"_qs);
			for (const auto& trackItem : Util::DomChildren (trackList, "item"_qs))
			{
				auto getText = [&trackItem] (const QString& elem)
				{
					const auto& text = trackItem.firstChildElement (elem).text ();
					return text.isEmpty () ? PlaylistManager::tr ("unknown") : text;
				};

				tracks.push_back ({
						.Url_ = QUrl::fromEncoded (getText ("playURL"_qs).toUtf8 ()),
						.Info_ = {
							.Artist_ = getText ("artistName"_qs),
							.Album_ = getText ("albumTitle"_qs),
							.Title_ = getText ("trackTitle"_qs),
							.Length_ = getText ("trackLength"_qs).toInt () / 1000,
						}
					});
			}

			co_return tracks;
		}

		Util::Task<Util::Either<QString, QList<Playlist>>> FetchPlaylists (QString accName, AuthManager *authMgr)
		{
			const auto sidResult = co_await authMgr->GetSID (accName);
			const auto sid = co_await sidResult;

			const auto& url = "https://ws.mp3tunes.com/api/v1/lockerData?output=xml&sid=%1&partner_token=%2&type=playlist"_qs
					.arg (sid, Consts::PartnerId);
			const auto response = co_await *GetProxyHolder ()->GetNetworkAccessManager ()->get (QNetworkRequest { url });
			const auto data = co_await response.ToEither ();
			const auto doc = co_await Util::AsDomDocument { data, PlaylistManager::tr ("unable to parse server response") };

			QList<Playlist> playlists;
			const auto& allPlaylistsElem = doc.documentElement ().firstChildElement ("playlistList"_qs);
			for (const auto& playlistElem : Util::DomChildren (allPlaylistsElem, "item"_qs))
			{
				const auto& id = playlistElem.firstChildElement ("playlistId"_qs).text ();
				if (id == u"INBOX"_qsv || id.isEmpty ())
					continue;

				FetchTracks (id, sid);
				playlists.push_back ({
							.Id_ = id,
							.Title_ = playlistElem.firstChildElement ("title"_qs).text (),
							.Tracks_ = co_await co_await FetchTracks (id, sid),
						});
			}
			co_return playlists;
		}
	}

	Util::ContextTask<> PlaylistManager::Update ()
	{
		co_await Util::AddContextObject { *this };

		while (Root_->rowCount ())
			Root_->removeRow (0);
		AccPlaylists_.clear ();
		Infos_.clear ();

		const auto& accs = AccMgr_->GetAccounts ();
		for (const auto& acc : accs)
		{
			auto item = new QStandardItem { acc };
			item->setEditable (false);
			Root_->appendRow (item);

			const auto result = co_await FetchPlaylists (acc, AuthMgr_);
			Visit (result,
					[&] (const QString& error)
					{
						const auto& e = Util::MakeNotification ("LMP"_qs,
								tr ("Unable to fetch playlists for %1: %2.").arg (acc, error),
								Priority::Critical);
						GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
					},
					[this, item] (const QList<Playlist>& playlists)
					{
						HandlePlaylists (*item, playlists);
					});
		}
	}

	std::optional<Media::AudioInfo> PlaylistManager::GetMediaInfo (const QUrl& url) const
	{
		return Infos_.contains (url) ?
				std::make_optional (Infos_ [url]) :
				std::optional<Media::AudioInfo> ();
	}

	void PlaylistManager::HandlePlaylists (QStandardItem& item, const QList<Playlist>& playlists)
	{
		QList<QStandardItem*> items;
		items.reserve (playlists.size ());
		for (const auto& playlist : playlists)
		{
			if (playlist.Tracks_.isEmpty ())
				continue;

			const auto item = new QStandardItem { playlist.Title_ };
			item->setEditable (false);
			items << item;

			QStringList compositions;
			QList<QUrl> urls;
			for (const auto& [url, info] : playlist.Tracks_)
			{
				Infos_ [url] = info;
				compositions << u"%1 — %2 — %3"_qs.arg (info.Artist_, info.Album_, info.Title_);
				urls << url;
			}
			item->setToolTip ("<ul><li>" + compositions.join (u"</li><li>"_qsv) + "</li></ul>");
			item->setData (QVariant::fromValue (urls), IPlaylistProvider::ItemRoles::SourceURLs);
		}
		item.appendRows (items);
	}
}

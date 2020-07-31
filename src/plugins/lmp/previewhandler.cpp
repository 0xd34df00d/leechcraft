/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "previewhandler.h"
#include <util/xpc/util.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/media/iaudiopile.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "player.h"
#include "core.h"
#include "previewcharacteristicinfo.h"

namespace LC
{
namespace LMP
{
	struct PreviewHandler::PendingTrackInfo
	{
		QString Artist_;
		QString Album_;
		QString Track_;
	};

	PreviewHandler::PreviewHandler (Player *player, QObject *parent)
	: QObject (parent)
	, Player_ (player)
	{
	}

	void PreviewHandler::InitWithPlugins ()
	{
		Providers_ = Core::Instance ().GetProxy ()->GetPluginsManager ()->GetAllCastableTo<Media::IAudioPile*> ();
	}

	void PreviewHandler::previewArtist (const QString& artist)
	{
		Media::AudioSearchRequest req;
		req.Artist_ = artist;
		RequestPreview (req);
	}

	void PreviewHandler::previewTrack (const QString& track, const QString& artist)
	{
		Media::AudioSearchRequest req;
		req.Title_ = track;
		req.Artist_ = artist;
		RequestPreview (req);
	}

	void PreviewHandler::previewTrack (const QString& track, const QString& artist, int length)
	{
		Media::AudioSearchRequest req;
		req.Title_ = track;
		req.Artist_ = artist;
		req.TrackLength_ = length;
		RequestPreview (req);
	}

	void PreviewHandler::previewAlbum (const QString& artist, const QString& album, const QList<QPair<QString, int>>& tracks)
	{
		Media::AudioSearchRequest req;
		req.Artist_ = artist;

		auto& storedAlbumTracks = Artist2Album2Tracks_ [artist] [album];

		for (const auto& pair : tracks)
		{
			req.Title_ = pair.first;
			req.TrackLength_ = pair.second;

			PendingTrackInfo info
			{
				artist,
				album,
				req.Title_
			};

			const auto& futures = RequestPreview (req);
			storedAlbumTracks [req.Title_] += futures.size ();

			for (const auto& future : futures)
				Util::Sequence (this, future) >>
						[this, info] (const auto& result)
						{
							const bool hasResults = Util::Visit (result,
									[] (const QString&) { return false; },
									[] (const auto& res) { return !res.isEmpty (); });
							CheckPendingAlbum (info, hasResults);
						};
		}
	}

	PreviewHandler::FuturesList_t PreviewHandler::RequestPreview (const Media::AudioSearchRequest& req)
	{
		auto handler = [this] (const Media::IAudioPile::Results_t& list) { HandlePendingReady (list); };

		QList<QFuture<Media::IAudioPile::Result_t>> futures;
		for (auto prov : Providers_)
		{
			auto future = prov->Search (req);
			Util::Sequence (this, future) >>
					Util::Visitor
					{
						[handler] (const QString&) { handler ({}); },
						handler
					};
			futures << future;
		}
		return futures;
	}

	/** Checks whether the given album is fully completed or not.
	 */
	void PreviewHandler::CheckPendingAlbum (const PendingTrackInfo& info, bool hasResults)
	{
		auto& tracks = Artist2Album2Tracks_ [info.Artist_] [info.Album_];

		/** If we don't have info.Track_ in our pending list it was fulfilled
		 * by another audio pile and we don't have to do anything.
		 */
		if (!tracks.contains (info.Track_))
			return;

		/** If this pending object has any results then it fulfills this track
		 * request, and we remove the track from the pending list. Otherwise
		 * we reduce the amount of available pending requests by one if this
		 * track hasn't been fulfilled yet.
		 */
		if (hasResults)
			tracks.remove (info.Track_);
		else if (tracks.contains (info.Track_))
			--tracks [info.Track_];

		/** Then we check if all interesting pending requests have finished.
		 * That is, if we have at least one track with non-zero pending count,
		 * or `tracks` is empty, we should either wait further or don't have to
		 * do anything. In case tracklist is empty we should clear
		 * Artist2Album2Tracks_.
		 */
		if (std::any_of (tracks.begin (), tracks.end (), [] (int c) { return c > 0; }))
			return;

		if (tracks.isEmpty ())
		{
			auto& artist = Artist2Album2Tracks_ [info.Artist_];
			artist.remove (info.Album_);
			if (artist.isEmpty ())
				Artist2Album2Tracks_.remove (info.Artist_);
			return;
		}

		/** Not all tracks were fulfilled and all pending requests have
		 * finished. We have to upset the user now :(
		 */
		const auto& e = Util::MakeNotification ("LMP",
				tr ("Not all tracks were fetched for album %1 by %2: %n track(s) weren't found.",
						0, tracks.size ())
					.arg (info.Album_)
					.arg (info.Artist_),
				Priority::Warning);
		Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}

	void PreviewHandler::HandlePendingReady (const Media::IAudioPile::Results_t& results)
	{
		QList<AudioSource> sources;
		QSet<QUrl> urls;
		QSet<PreviewCharacteristicInfo> infos;
		for (const auto& res : results)
		{
			if (urls.contains (res.Source_))
				continue;
			urls.insert (res.Source_);

			const PreviewCharacteristicInfo checkInfo { res.Info_ };
			if (infos.contains (checkInfo))
				continue;
			infos << checkInfo;

			Player_->PrepareURLInfo (res.Source_, MediaInfo::FromAudioInfo (res.Info_));
			sources << AudioSource (res.Source_);
		}

		if (!sources.isEmpty ())
			Player_->Enqueue (sources, Player::EnqueueNone);
	}
}
}

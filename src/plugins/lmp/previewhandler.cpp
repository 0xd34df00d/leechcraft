/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "previewhandler.h"
#include <util/util.h>
#include <interfaces/media/iaudiopile.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "player.h"
#include "core.h"

namespace LeechCraft
{
namespace LMP
{
	PreviewHandler::PreviewHandler (Player *player, QObject *parent)
	: QObject (parent)
	, Player_ (player)
	{
		Providers_ = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableTo<Media::IAudioPile*> ();
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

			PendingTrackInfo info =
			{
				artist,
				album,
				req.Title_
			};

			const auto& pendings = RequestPreview (req);
			for (const auto& pending : pendings)
				Pending2Track_ [pending] = info;

			storedAlbumTracks [req.Title_] += pendings.size ();
		}
	}

	QList<Media::IPendingAudioSearch*> PreviewHandler::RequestPreview (const Media::AudioSearchRequest& req)
	{
		QList<Media::IPendingAudioSearch*> pendings;
		for (auto prov : Providers_)
		{
			auto pending = prov->Search (req);
			connect (pending->GetObject (),
					SIGNAL (ready ()),
					this,
					SLOT (handlePendingReady ()));
			pendings << pending;
		}
		return pendings;
	}

	/** Checks whether the given album is fully completed or not.
	 */
	void PreviewHandler::CheckPendingAlbum (Media::IPendingAudioSearch *pending)
	{
		if (!Pending2Track_.contains (pending))
			return;

		const auto& info = Pending2Track_.take (pending);

		auto& tracks = Artist2Album2Tracks_ [info.Artist_] [info.Album_];

		/** If we don't have info.Track_ in our pending list it was fulfilled
		 * by another audio pile and we don't have to do anything.
		 */
		if (!tracks.contains (info.Track_))
			return;

		/** If this pending object has any results then it fulfills this track
		 * request, and we remove the track from the pending list. Otherwise
		 * we reduce the amount of available pending requests by one.
		 */
		if (!pending->GetResults ().isEmpty ())
			tracks.remove (info.Track_);
		else
			--tracks [info.Track_];

		/** Then we check if all interesting pending requests have finished.
		 * That is, if we have at least one track with non-zero pending count,
		 * or `tracks` is empty, we should either wait further or don't have to
		 * do anything. In case tracklist is empty we should clear
		 * Artist2Album2Tracks_.
		 */
		if (std::find_if (tracks.begin (), tracks.end (),
				[] (int c) { return c > 0; }) != tracks.end ())
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
				PWarning_);
		Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}

	namespace
	{
		struct SamenessCheckInfo
		{
			QString Artist_;
			QString Title_;
			qint32 Length_;
		};

		bool operator== (const SamenessCheckInfo& i1, const SamenessCheckInfo& i2)
		{
			return i1.Artist_ == i2.Artist_ &&
					i1.Title_ == i2.Title_ &&
					i1.Length_ == i2.Length_;
		}

		uint qHash (const SamenessCheckInfo& info)
		{
			return qHash (info.Artist_ + '|' + info.Title_ + '|' + QString::number (info.Length_));
		}
	}

	void PreviewHandler::handlePendingReady ()
	{
		auto pending = qobject_cast<Media::IPendingAudioSearch*> (sender ());

		QList<Phonon::MediaSource> sources;
		QSet<QUrl> urls;
		QSet<SamenessCheckInfo> infos;
		for (const auto& res : pending->GetResults ())
		{
			if (urls.contains (res.Source_))
				continue;
			urls.insert (res.Source_);

			const SamenessCheckInfo checkInfo
			{
				res.Info_.Album_.toLower ().trimmed (),
				res.Info_.Title_.toLower ().trimmed (),
				res.Info_.Length_
			};
			if (infos.contains (checkInfo))
				continue;
			infos << checkInfo;

			Player_->PrepareURLInfo (res.Source_, MediaInfo::FromAudioInfo (res.Info_));
			sources << Phonon::MediaSource (res.Source_);
		}

		if (!sources.isEmpty ())
			Player_->Enqueue (sources, false);

		CheckPendingAlbum (pending);
	}
}
}

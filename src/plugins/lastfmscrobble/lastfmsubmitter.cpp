/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
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

#include "lastfmsubmitter.h"
#include <QCryptographicHash>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QTimer>
#include <lastfm/Track>
#include <lastfm.h>
#include <interfaces/media/audiostructs.h>
#include <util/util.h>
#include "util.h"
#include "codes.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	MediaMeta::MediaMeta ()
	: TrackNumber_ (0)
	, Length_ (0)
	{
	}

	MediaMeta::MediaMeta (const QMap<QString, QVariant>& tagMap)
	: Artist_ (tagMap ["Artist"].toString ())
	, Album_ (tagMap ["Album"].toString ())
	, Title_ (tagMap ["Title"].toString ())
	, Genre_ (tagMap ["Genre"].toString ())
	, Date_ (tagMap ["Date"].toString ())
	, TrackNumber_ (tagMap ["TrackNumber"].toInt ())
	, Length_ (tagMap ["Length"].toInt ())
	{
	}

	MediaMeta::MediaMeta (const Media::AudioInfo& info)
	: Artist_ (info.Artist_)
	, Album_ (info.Album_)
	, Title_ (info.Title_)
	, Genre_ (info.Genres_.join (" / "))
	, Date_ (QString::number (info.Year_))
	, TrackNumber_ (info.TrackNumber_)
	, Length_ (info.Length_)
	{
	}

	namespace
	{
		QString GetQueueFilename ()
		{
			return Util::CreateIfNotExists ("lastfmscrobble").absoluteFilePath ("queue.xml");
		}
	}

	LastFMSubmitter::LastFMSubmitter (QObject *parent)
	: QObject (parent)
	, SubmitTimer_ (new QTimer (this))
	{
		lastfm::ws::ApiKey = "a5ca8821e39cdb5efd2e5667070084b2";
		lastfm::ws::SharedSecret = "50fb8b6f35fc55b7ddf6bb033dfc6fbe";

		SubmitTimer_->setSingleShot (true);
		connect (SubmitTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (cacheAndSubmit ()),
				Qt::UniqueConnection);

		LoadQueue ();
	}

	void LastFMSubmitter::Init (QNetworkAccessManager *manager)
	{
		NAM_ = manager;
	}

	bool LastFMSubmitter::IsConnected () const
	{
		return Scrobbler_ ? true : false;
	}

	namespace
	{
		lastfm::MutableTrack ToLastFMTrack (const MediaMeta& info)
		{
			lastfm::MutableTrack mutableTrack;
			mutableTrack.setTitle (info.Title_);
			mutableTrack.setAlbum (info.Album_);
			mutableTrack.setArtist (info.Artist_);
			mutableTrack.stamp ();
			mutableTrack.setSource (lastfm::Track::Player);
			mutableTrack.setDuration (info.Length_);
			mutableTrack.setTrackNumber (info.TrackNumber_);
			return mutableTrack;
		}
	}

	void LastFMSubmitter::NowPlaying (const MediaMeta& info)
	{
		SubmitTimer_->stop ();

		NextSubmit_ = lastfm::Track ();
		if (info.Length_ < 30)
			return;

		const auto& lfmTrack = ToLastFMTrack (info);
		if (!Scrobbler_)
		{
			SubmitQueue_ << lfmTrack;
			return;
		}
		Scrobbler_->nowPlaying (lfmTrack);

		NextSubmit_ = lfmTrack;
		SubmitTimer_->start (std::min (info.Length_ / 2, 240) * 1000);
	}

	void LastFMSubmitter::Love ()
	{
		if (NextSubmit_.isNull ())
			return;

		QList<QPair<QString, QString>> params;
		params << QPair<QString, QString> ("track", NextSubmit_.title ());
		params << QPair<QString, QString> ("artist", NextSubmit_.artist ());
		qDebug () << Q_FUNC_INFO << "loving" << NextSubmit_.artist () << NextSubmit_.title ();
		QNetworkReply *reply = Request ("track.love", NAM_, params);
		connect (reply,
				SIGNAL (finished ()),
				reply,
				SLOT (deleteLater ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				reply,
				SLOT (deleteLater ()));
	}

	void LastFMSubmitter::Clear ()
	{
		NextSubmit_ = lastfm::MutableTrack ();
		SubmitTimer_->stop ();
	}

	void LastFMSubmitter::LoadQueue ()
	{
		QFile file (GetQueueFilename ());
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< file.errorString ();
			return;
		}

		QDomDocument doc;
		doc.setContent (file.readAll ());
		auto elem = doc.documentElement ().firstChildElement ();
		while (!elem.isNull ())
		{
			lastfm::Track track (elem);
			if (!track.isNull ())
				SubmitQueue_ << track;
			elem = elem.nextSiblingElement ();
		}
	}

	void LastFMSubmitter::SaveQueue () const
	{
		QFile file (GetQueueFilename ());
		if (SubmitQueue_.isEmpty ())
		{
			file.remove ();
			return;
		}

		QDomDocument doc ("queue");
		auto root = doc.createElement ("queue");
		doc.appendChild (root);

		std::for_each (SubmitQueue_.begin (), SubmitQueue_.end (),
				[&doc, &root] (decltype (SubmitQueue_.front ()) item)
				{
					if (item.duration ())
						root.appendChild (item.toDomElement (doc));
				});

		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< file.errorString ();
			return;
		}

		file.write (doc.toByteArray ());
	}

	void LastFMSubmitter::handleAuthenticated ()
	{
		Scrobbler_.reset (new lastfm::Audioscrobbler ("tst"));

		connect (Scrobbler_.get (),
				SIGNAL (status (int)),
				this,
				SIGNAL (status (int)));
		connect (Scrobbler_.get (),
				SIGNAL (status (int)),
				this,
				SLOT (checkFlushQueue (int)));

		if (!SubmitQueue_.isEmpty ())
		{
			Scrobbler_->cache (SubmitQueue_);
			submit ();
		}
	}

	void LastFMSubmitter::cacheAndSubmit ()
	{
		Scrobbler_->cache (NextSubmit_);
		submit ();
	}

	void LastFMSubmitter::checkFlushQueue (int code)
	{
		qDebug () << Q_FUNC_INFO << code;
		if (code == lastfm::Audioscrobbler::TracksScrobbled || code == lastfm::Audioscrobbler::Scrobbling)
		{
			qDebug () << "tracks scrobbled, clearing queue";
			SubmitQueue_.clear ();
			SaveQueue ();
		}
	}

	void LastFMSubmitter::submit ()
	{
		SubmitQueue_ << NextSubmit_;
		SaveQueue ();

		if (!IsConnected ())
			return;

		Scrobbler_->submit ();
	}
}
}

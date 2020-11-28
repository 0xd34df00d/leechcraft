/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lastfmsubmitter.h"
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QTimer>
#include <interfaces/media/audiostructs.h>
#include <util/util.h>
#include <util/sll/prelude.h>
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	LastFMSubmitter::LastFMSubmitter (QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
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
	}

	bool LastFMSubmitter::IsConnected () const
	{
		return static_cast<bool> (Scrobbler_);
	}

	namespace
	{
		lastfm::MutableTrack ToLastFMTrack (const Media::AudioInfo& info)
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

	void LastFMSubmitter::NowPlaying (const Media::AudioInfo& info)
	{
		SubmitTimer_->stop ();

		if (!NextSubmit_.isNull ())
		{
			const int secsTo = NextSubmit_.timestamp ().secsTo (QDateTime::currentDateTime ());
			if (!NextSubmit_.duration () && secsTo > 30)
			{
				NextSubmit_.setDuration (secsTo);
				if (Scrobbler_)
					cacheAndSubmit ();
			}
			else
				NextSubmit_ = lastfm::Track ();
		}

		if (info.Length_ && info.Length_ < 30)
			return;

		const auto& lfmTrack = ToLastFMTrack (info);
		if (!Scrobbler_)
			return;
		Scrobbler_->nowPlaying (lfmTrack);

		NextSubmit_ = lfmTrack;
		if (info.Length_)
			SubmitTimer_->start (std::min (info.Length_ / 2, 240) * 1000);
	}

	void LastFMSubmitter::SendBackdated (const QList<QPair<Media::AudioInfo, QDateTime>>& backdateList)
	{
		const auto& submissions = Util::Map (backdateList,
				[] (const auto& pair) -> lastfm::Track
				{
					auto track = ToLastFMTrack (pair.first);
					track.setTimeStamp (pair.second);
					return track;
				});

		Scrobbler_->cacheBatch (submissions);
		Scrobbler_->submit ();
	}

	namespace
	{
		QList<std::pair<QString, QString>> MakeLoveBanParams (const lastfm::MutableTrack& track)
		{
			return
			{
				{ "track", track.title () },
				{ "artist", track.artist () }
			};
		}
	}

	void LastFMSubmitter::Love ()
	{
		if (NextSubmit_.isNull ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no track in submit queue, can't make love";
			return;
		}

		const auto reply = Request ("track.love", NAM_, MakeLoveBanParams (NextSubmit_));
		connect (reply,
				SIGNAL (finished ()),
				reply,
				SLOT (deleteLater ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				reply,
				SLOT (deleteLater ()));
	}

	void LastFMSubmitter::Ban ()
	{
		if (NextSubmit_.isNull ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no track in submit queue, can't ban";
			return;
		}

		const auto reply = Request ("track.ban", NAM_, MakeLoveBanParams (NextSubmit_));
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

	void LastFMSubmitter::handleAuthenticated ()
	{
		Scrobbler_.reset (new lastfm::Audioscrobbler ("tst"));

		connect (Scrobbler_.get (),
				SIGNAL (nowPlayingError (int, QString)),
				this,
				SLOT (handleNPError (int, QString)));
	}

	void LastFMSubmitter::handleNPError (int code, const QString& msg)
	{
		qWarning () << Q_FUNC_INFO
				<< code
				<< msg;
	}

	void LastFMSubmitter::cacheAndSubmit ()
	{
		Scrobbler_->cache (NextSubmit_);
		submit ();
		NextSubmit_ = lastfm::Track ();
	}

	void LastFMSubmitter::submit ()
	{
		if (!IsConnected ())
			return;

		Scrobbler_->submit ();
	}
}
}

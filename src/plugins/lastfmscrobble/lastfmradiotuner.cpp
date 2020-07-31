/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lastfmradiotuner.h"
#include <QtDebug>
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	LastFmRadioTuner::LastFmRadioTuner (const lastfm::RadioStation& station,
			QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, NumTries_ (0)
	{
		auto reply = Request ("radio.tune", NAM_, ParamsList_t { { "station", station.url () }});
		connect (reply,
				&QNetworkReply::finished,
				reply,
				&QObject::deleteLater);
	}

	lastfm::Track LastFmRadioTuner::GetNextTrack ()
	{
		lastfm::Track result;
		if (!Queue_.isEmpty ())
			result = Queue_.takeFirst ();
		if (Queue_.isEmpty ())
			FetchMoreTracks ();
		return result;
	}

	void LastFmRadioTuner::FetchMoreTracks ()
	{
		auto reply = Request ("radio.getPlaylist", NAM_, ParamsList_t { { "rtp", "1" }});
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotPlaylist ()));
	}

	bool LastFmRadioTuner::TryAgain ()
	{
		if (++NumTries_ > 5)
			return false;

		FetchMoreTracks ();
		return true;
	}

	void LastFmRadioTuner::handleGotPlaylist ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		qDebug () << data;
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing playlist";
			emit error ("Parse error");
		}

		const auto& docElem = doc.documentElement ();
		if (docElem.attribute ("status") == "failed")
		{
			const auto& errElem = docElem.firstChildElement ("error");
			qWarning () << Q_FUNC_INFO
					<< errElem.text ();
			emit error (errElem.text ());
			return;
		}

		try
		{
			auto tracks = lastfm::Xspf { doc.documentElement ().firstChildElement ("playlist"), this }.tracks ();
			if (tracks.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO << "no tracks";
				throw lastfm::ws::TryAgainLater;
			}

			NumTries_ = 0;

			for (const auto& track : tracks)
				lastfm::MutableTrack (track).setSource (lastfm::Track::LastFmRadio);

			Queue_ += tracks;
			emit trackAvailable ();
		}
		catch (const lastfm::ws::ParseError& e)
		{
			qWarning () << Q_FUNC_INFO << e.message ();
			if (e.enumValue () != lastfm::ws::TryAgainLater)
				emit error (e.message ());
			if (!TryAgain ())
				emit error ("out of tries");
		}
		catch (const lastfm::ws::Error& e)
		{
			qWarning () << Q_FUNC_INFO << e;
		}
	}
}
}

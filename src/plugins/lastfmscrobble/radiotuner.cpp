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

#include "radiotuner.h"
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	RadioTuner::RadioTuner (const lastfm::RadioStation& station,
			QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, NumTries_ (0)
	{
		QList<QPair<QString, QString>> params;
		params << QPair<QString, QString> ("station", station.url ());
		auto reply = Request ("radio.tune", NAM_, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleTuned ()));
	}

	lastfm::Track RadioTuner::GetNextTrack ()
	{
		lastfm::Track result;
		if (!Queue_.isEmpty ())
			result = Queue_.takeFirst ();
		if (Queue_.isEmpty ())
			FetchMoreTracks ();
		return result;
	}

	void RadioTuner::FetchMoreTracks ()
	{
		QList<QPair<QString, QString>> params;
		params << QPair<QString, QString> ("rtp", "1");
		auto reply = Request ("radio.getPlaylist", NAM_, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotPlaylist ()));
	}

	bool RadioTuner::TryAgain ()
	{
		if (++NumTries_ > 5)
			return false;

		FetchMoreTracks ();
		return true;
	}

	void RadioTuner::handleTuned ()
	{
		sender ()->deleteLater ();
	}

	void RadioTuner::handleGotPlaylist ()
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
			lastfm::Xspf xspf (doc.documentElement ().firstChildElement ("playlist"));
			auto tracks = xspf.tracks ();

			if (tracks.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO << "no tracks";
				throw lastfm::ws::TryAgainLater;
			}

			NumTries_ = 0;

			Q_FOREACH (auto track, tracks)
				lastfm::MutableTrack (track).setSource (lastfm::Track::LastFmRadio);

			Queue_ += tracks;
			emit trackAvailable ();
		}
		catch (const lastfm::ws::ParseError& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			if (e.enumValue () != lastfm::ws::TryAgainLater)
				emit error (e.what ());
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

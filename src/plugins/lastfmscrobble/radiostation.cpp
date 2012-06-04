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

#include "radiostation.h"
#include <QtDebug>
#include <lastfm/RadioStation>
#include "radiotuner.h"
#include <interfaces/media/audiostructs.h>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	RadioStation::RadioStation (QNetworkAccessManager *nam,
			Media::IRadioStationProvider::Type type, const QString& param)
	{
		lastfm::RadioStation station;
		switch (type)
		{
		case Media::IRadioStationProvider::Type::SimilarArtists:
			station = lastfm::RadioStation::similar (lastfm::Artist (param));
			break;
		case Media::IRadioStationProvider::Type::GlobalTag:
			station = lastfm::RadioStation::globalTag (lastfm::Tag (param));
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unsupported type"
					<< static_cast<int> (type);
			throw UnsupportedType ();
		}

		Tuner_.reset (new RadioTuner (station, nam));

		connect (Tuner_.get (),
				SIGNAL (error (QString)),
				this,
				SLOT (handleError (QString)));
		connect (Tuner_.get (),
				SIGNAL (trackAvailable ()),
				this,
				SLOT (handleNextTrack ()));
	}

	QObject* RadioStation::GetObject ()
	{
		return this;
	}

	void RadioStation::RequestNewStream ()
	{
		auto track = Tuner_->GetNextTrack ();
		if (!track.isNull ())
		{
			EmitTrack (track);
			return;
		}

		connect (Tuner_.get (),
				SIGNAL (trackAvailable ()),
				this,
				SLOT (handleNextTrack ()));
	}

	void RadioStation::EmitTrack (const lastfm::Track& track)
	{
		qDebug () << Q_FUNC_INFO << track.url ();
		const Media::AudioInfo info =
		{
			track.artist (),
			track.album (),
			track.title (),
			QStringList (),
			track.duration () / 1000,
			0,
			track.trackNumber ()
		};
		emit gotNewStream (track.url (), info);
	}

	void RadioStation::handleTitle (const QString& title)
	{
		qDebug () << Q_FUNC_INFO << title;
	}

	void RadioStation::handleError (const QString& error)
	{
		qDebug () << Q_FUNC_INFO << error;
		emit gotError (error);
	}

	void RadioStation::handleNextTrack ()
	{
		EmitTrack (Tuner_->GetNextTrack ());

		disconnect (Tuner_.get (),
				SIGNAL (trackAvailable ()),
				this,
				SLOT (handleNextTrack ()));
	}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <interfaces/media/audiostructs.h>
#include "xmlsettingsmanager.h"
#include "radiotuner.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	QMap<QByteArray, QString> RadioStation::GetPredefinedStations ()
	{
		const auto& login = XmlSettingsManager::Instance ().property ("lastfm.login").toString ();

		QMap<QByteArray, QString> result;
		if (!login.isEmpty ())
		{
			result ["library"] = tr ("Library");
			result ["recommendations"] = tr ("Recommendations");
			result ["loved"] = tr ("Loved tracks");
			result ["neighbourhood"] = tr ("Neighbourhood");
		}
		return result;
	}

	RadioStation::RadioStation (QNetworkAccessManager *nam,
			Media::RadioType type, const QString& param, const QString& visibleName)
	{
		lastfm::RadioStation station;
		switch (type)
		{
		case Media::RadioType::SimilarArtists:
			station = lastfm::RadioStation::similar (lastfm::Artist (param));
			RadioName_ = tr ("Similar to \"%1\" radio").arg (param);
			break;
		case Media::RadioType::GlobalTag:
#if LASTFM_VERSION < 0x00010000
			station = lastfm::RadioStation::globalTag (lastfm::Tag (param));
#else
			station = lastfm::RadioStation::tag (lastfm::Tag (param));
#endif
			RadioName_ = tr ("Tag \"%1\" radio").arg (param);
			break;
		case Media::RadioType::Predefined:
		{
			const auto& login = XmlSettingsManager::Instance ().property ("lastfm.login").toString ();
			const lastfm::User user (login);

			if (param == "library")
				station = lastfm::RadioStation::library (user);
			else if (param == "recommendations")
				station = lastfm::RadioStation::recommendations (user);
			else if (param == "loved")
#if LASTFM_VERSION < 0x00010000
				station = lastfm::RadioStation::lovedTracks (user);
#else
				station = lastfm::RadioStation::mix (user);
#endif
			else if (param == "neighbourhood")
				station = lastfm::RadioStation::neighbourhood (user);

			RadioName_ = visibleName;

			break;
		}
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

	QString RadioStation::GetRadioName () const
	{
		return RadioName_;
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
			static_cast<qint32> (track.duration () / 1000),
			0,
			static_cast<qint32> (track.trackNumber ()),
			QVariantMap ()
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

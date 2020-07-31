/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lastfmradiostation.h"
#include <QtDebug>
#include <interfaces/media/audiostructs.h>
#include "xmlsettingsmanager.h"
#include "lastfmradiotuner.h"

namespace LC
{
namespace Lastfmscrobble
{
	QMap<QByteArray, QString> LastFmRadioStation::GetPredefinedStations ()
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

	LastFmRadioStation::LastFmRadioStation (QNetworkAccessManager *nam,
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
			station = lastfm::RadioStation::tag (lastfm::Tag (param));
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
				station = lastfm::RadioStation::mix (user);
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

		Tuner_ = std::make_shared<LastFmRadioTuner> (station,  nam);

		connect (Tuner_.get (),
				SIGNAL (error (QString)),
				this,
				SLOT (handleError (QString)));
		connect (Tuner_.get (),
				SIGNAL (trackAvailable ()),
				this,
				SLOT (handleNextTrack ()));
	}

	QObject* LastFmRadioStation::GetQObject ()
	{
		return this;
	}

	void LastFmRadioStation::RequestNewStream ()
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

	QString LastFmRadioStation::GetRadioName () const
	{
		return RadioName_;
	}

	void LastFmRadioStation::EmitTrack (const lastfm::Track& track)
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

	void LastFmRadioStation::handleTitle (const QString& title)
	{
		qDebug () << Q_FUNC_INFO << title;
	}

	void LastFmRadioStation::handleError (const QString& error)
	{
		qDebug () << Q_FUNC_INFO << error;
		emit gotError (error);
	}

	void LastFmRadioStation::handleNextTrack ()
	{
		EmitTrack (Tuner_->GetNextTrack ());

		disconnect (Tuner_.get (),
				SIGNAL (trackAvailable ()),
				this,
				SLOT (handleNextTrack ()));
	}
}
}

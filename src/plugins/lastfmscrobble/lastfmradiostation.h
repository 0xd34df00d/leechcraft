/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/media/iradiostation.h>
#include <interfaces/media/iradiostationprovider.h>
#include "lastfmheaders.h"

class QNetworkAccessManager;

namespace LC
{
namespace Lastfmscrobble
{
	class LastFmRadioTuner;

	class LastFmRadioStation : public QObject
					   , public Media::IRadioStation
	{
		Q_OBJECT
		Q_INTERFACES (Media::IRadioStation)

		std::shared_ptr<LastFmRadioTuner> Tuner_;
		QString RadioName_;
	public:
		struct UnsupportedType {};

		static QMap<QByteArray, QString> GetPredefinedStations ();

		LastFmRadioStation (QNetworkAccessManager*,
				Media::RadioType,
				const QString& param,
				const QString& visibleName);

		QObject* GetQObject ();
		void RequestNewStream ();
		QString GetRadioName () const;
	private:
		void EmitTrack (const lastfm::Track&);
	private slots:
		void handleTitle (const QString&);
		void handleError (const QString&);
		void handleNextTrack ();
	signals:
		void gotPlaylist (const QString&, const QString&);
		void gotNewStream (const QUrl&, const Media::AudioInfo&);
		void gotAudioInfos (const QList<Media::AudioInfo>& infos);
		void gotError (const QString&);
	};
}
}

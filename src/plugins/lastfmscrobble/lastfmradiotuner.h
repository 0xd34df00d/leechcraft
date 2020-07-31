/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "lastfmheaders.h"

class QNetworkAccessManager;

namespace lastfm
{
	class RadioStation;
}

namespace LC
{
namespace Lastfmscrobble
{
	class LastFmRadioTuner : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;
		QList<lastfm::Track> Queue_;
		int NumTries_;
	public:
		LastFmRadioTuner (const lastfm::RadioStation&, QNetworkAccessManager*, QObject* = 0);

		lastfm::Track GetNextTrack ();
	private:
		void FetchMoreTracks ();
		bool TryAgain ();
	private slots:
		void handleGotPlaylist ();
	signals:
		void error (const QString&);
		void trackAvailable ();
	};
}
}

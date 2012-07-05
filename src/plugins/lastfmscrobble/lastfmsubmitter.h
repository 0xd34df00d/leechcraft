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

#pragma once

#include <memory>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QMap>
#include "lastfmheaders.h"

class QTimer;

namespace lastfm
{
	class Audioscrobbler;
}

namespace Media
{
	struct AudioInfo;
}

class QNetworkAccessManager;

namespace LeechCraft
{
namespace Lastfmscrobble
{
	struct MediaMeta
	{
		QString Artist_, Album_, Title_, Genre_, Date_;
		int TrackNumber_;
		int Length_;

		MediaMeta ();
		explicit MediaMeta (const QMap<QString, QVariant>& tagMap);
		explicit MediaMeta (const Media::AudioInfo& tagMap);
	};

	class LastFMSubmitter : public QObject
	{
		Q_OBJECT

		std::shared_ptr<lastfm::Audioscrobbler> Scrobbler_;

		QNetworkAccessManager *NAM_;

		QTimer *SubmitTimer_;

		lastfm::MutableTrack NextSubmit_;
	public:
		LastFMSubmitter (QObject *parent = 0);

		void Init (QNetworkAccessManager *manager);
		bool IsConnected () const;

		void NowPlaying (const MediaMeta&);
		void Love ();
		void Clear ();
	public slots:
		void submit ();
		void handleAuthenticated ();
	private slots:
		void cacheAndSubmit ();
		void checkFlushQueue (int);
	signals:
		void status (int code);
		void authFailure ();
	};
}
}

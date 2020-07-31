/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

namespace LC
{
namespace Lastfmscrobble
{
	class LastFMSubmitter : public QObject
	{
		Q_OBJECT

		std::shared_ptr<lastfm::Audioscrobbler> Scrobbler_;

		QNetworkAccessManager * const NAM_;

		QTimer *SubmitTimer_;

		lastfm::MutableTrack NextSubmit_;
	public:
		LastFMSubmitter (QNetworkAccessManager*, QObject *parent = nullptr);

		bool IsConnected () const;

		void NowPlaying (const Media::AudioInfo&);
		void SendBackdated (const QList<QPair<Media::AudioInfo, QDateTime>>&);
		void Love ();
		void Ban ();
		void Clear ();
	public slots:
		void submit ();
		void handleAuthenticated ();
	private slots:
		void handleNPError (int, const QString&);
		void cacheAndSubmit ();
	signals:
		void status (int code);
		void authFailure ();
	};
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/media/ieventsprovider.h>

class QNetworkAccessManager;

namespace LC
{
namespace Lastfmscrobble
{
	class Authenticator;

	class EventAttendMarker : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;
		qint64 ID_;
		int Code_ = 0;
	public:
		EventAttendMarker (Authenticator*, QNetworkAccessManager*, qint64, Media::EventAttendType, QObject* = 0);
	private slots:
		void mark ();
		void handleFinished ();
		void handleError ();
	signals:
		void finished ();
	};
}
}

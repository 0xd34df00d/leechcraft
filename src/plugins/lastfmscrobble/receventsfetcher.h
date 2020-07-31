/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <interfaces/media/ieventsprovider.h>

class QNetworkAccessManager;

namespace LC
{
namespace Lastfmscrobble
{
	class Authenticator;

	class RecEventsFetcher : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager *NAM_;
	public:
		enum class Type
		{
			Attending,
			Recommended
		};
	private:
		Type Type_;
	public:
		RecEventsFetcher (Authenticator*, QNetworkAccessManager*, Type, QObject* = 0);
	private:
		void RequestEvents (QMap<QString, QString>);
	private slots:
		void request ();
		void handleLocationReceived ();
		void handleLocationError ();
		void handleFinished ();
		void handleError ();
	signals:
		void gotRecommendedEvents (const Media::EventInfos_t&);
	};
}
}

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

#pragma once

#include <QObject>
#include <QMap>
#include <interfaces/media/ieventsprovider.h>

class QNetworkAccessManager;

namespace LeechCraft
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

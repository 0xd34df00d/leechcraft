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

#include "hypedtracksfetcher.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QtDebug>
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	HypedTracksFetcher::HypedTracksFetcher (QNetworkAccessManager *nam, Media::IHypesProvider::HypeType type, QObject *parent)
	: QObject (parent)
	, Type_ (type)
	{
		QMap<QString, QString> params;
		params ["limit"] = "50";
		auto reply = Request ("chart.getHypedTracks", nam, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));
	}

	void HypedTracksFetcher::handleFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply"
					<< data;
			return;
		}

		QList<Media::HypedTrackInfo> tracks;

		auto trackElem = doc
				.documentElement ()
				.firstChildElement ("tracks")
				.firstChildElement ("track");
		while (!trackElem.isNull ())
		{
			auto getText = [&trackElem] (const QString& name)
			{
				return trackElem.firstChildElement (name).text ();
			};

			const auto& artistElem = trackElem.firstChildElement ("artist");

			tracks << Media::HypedTrackInfo
			{
				getText ("name"),
				getText ("url"),
				getText ("percentagechange").toInt (),
				getText ("duration").toInt (),
				GetImage (trackElem, "medium"),
				GetImage (trackElem, "extralarge"),
				artistElem.firstChildElement ("name").text (),
				artistElem.firstChildElement ("url").text ()
			};

			trackElem = trackElem.nextSiblingElement ("track");
		}

		emit gotHypedTracks (tracks, Type_);
	}

	void HypedTracksFetcher::handleError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		reply->deleteLater ();
		deleteLater ();
	}
}
}

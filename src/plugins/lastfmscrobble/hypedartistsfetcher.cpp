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

#include "hypedartistsfetcher.h"
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
	HypedArtistsFetcher::HypedArtistsFetcher (QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	{
		qDebug () << Q_FUNC_INFO;
		auto reply = Request ("chart.getHypedArtists", nam);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));
	}

	void HypedArtistsFetcher::handleFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		deleteLater ();

		const auto& data = reply->readAll ();
		qDebug () << data;
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply"
					<< data;
			return;
		}

		QList<Media::HypedArtistInfo> infos;

		auto artistElem = doc
				.documentElement ()
				.firstChildElement ("artists")
				.firstChildElement ("artist");
		while (!artistElem.isNull ())
		{
			auto getText = [&artistElem] (const QString& name)
			{
				return artistElem.firstChildElement (name).text ();
			};

			infos << Media::HypedArtistInfo
			{
				Media::ArtistInfo
				{
					getText ("name"),
					QString (),
					QString (),
					GetImage (artistElem, "medium"),
					GetImage (artistElem, "extralarge"),
					getText ("url"),
					Media::TagInfos_t ()
				},
				getText ("percentagechange").toInt ()
			};
			artistElem = artistElem.nextSiblingElement ("artist");
		}

		emit gotHypedArtists (infos);
	}

	void HypedArtistsFetcher::handleError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		reply->deleteLater ();
		deleteLater ();
	}
}
}

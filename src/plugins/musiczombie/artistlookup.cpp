/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "artistlookup.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QtDebug>

namespace LeechCraft
{
namespace MusicZombie
{
	ArtistLookup::ArtistLookup (const QString& name, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	{
		QUrl url ("http://www.musicbrainz.org/ws/2/artist/");
		url.addQueryItem ("query", "artist:" + name);
		auto reply = nam->get (QNetworkRequest (url));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL(error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));
	}

	void ArtistLookup::handleFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply"
					<< data;
			emit replyError ();
			return;
		}

		const auto& artists = doc.documentElement ().firstChildElement ("artist-list");
		if (artists.attribute ("count").toInt () < 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "no such artists";
			emit replyError ();
			return;
		}

		const auto& artist = artists.firstChildElement ("artist");
		const auto& id = artist.attribute ("id");
		emit gotID (id);
	}

	void ArtistLookup::handleError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();
		emit networkError ();
	}
}
}

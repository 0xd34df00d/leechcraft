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

#include "pendingartistbio.h"
#include <algorithm>
#include <QNetworkReply>
#include <QDomDocument>
#include <QtDebug>
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	PendingArtistBio::PendingArtistBio (const QString& name,
			QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	{
		QMap<QString, QString> params;
		params ["artist"] = name;
		params ["autocorrect"] = "1";
		AddLanguageParam (params);
		auto reply = Request ("artist.getInfo", nam, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));
	}

	QObject* PendingArtistBio::GetObject ()
	{
		return this;
	}

	Media::ArtistBio PendingArtistBio::GetArtistBio () const
	{
		return Bio_;
	}

	void PendingArtistBio::handleFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		QDomDocument doc;
		if (!doc.setContent (reply->readAll ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply";
			emit error ();
			deleteLater ();
			return;
		}

		const auto& artist = doc.documentElement ().firstChildElement ("artist");
		Bio_.BasicInfo_ = GetArtistInfo (artist);
		std::reverse (Bio_.BasicInfo_.Tags_.begin (), Bio_.BasicInfo_.Tags_.end ());

		emit ready ();
		deleteLater ();
	}

	void PendingArtistBio::handleError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();
		reply->deleteLater ();
		deleteLater ();

		emit error ();
	}
}
}

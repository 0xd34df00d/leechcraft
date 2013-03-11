/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "pendingrecommendedartists.h"
#include <memory>
#include <QDomDocument>
#include <QNetworkReply>
#include <QtDebug>
#include "authenticator.h"
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	PendingRecommendedArtists::PendingRecommendedArtists (Authenticator *auth,
			QNetworkAccessManager *nam, int num, QObject *obj)
	: BaseSimilarArtists (QString (), num, obj)
	, NAM_ (nam)
	{
		if (auth->IsAuthenticated ())
			request ();
		else
			connect (auth,
					SIGNAL (authenticated ()),
					this,
					SLOT (request ()));
	}

	void PendingRecommendedArtists::request ()
	{
		QList<QPair<QString, QString>> params;
		params << QPair<QString, QString> ("limit", QString::number (NumGet_));
		auto reply = Request ("user.getRecommendedArtists", NAM_, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleReplyError ()));
	}

	void PendingRecommendedArtists::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		QDomDocument doc;
		if (!doc.setContent (reply->readAll ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply";
			emit error ();
			return;
		}

		auto artistElem = doc.documentElement ()
				.firstChildElement ("recommendations")
				.firstChildElement ("artist");
		auto elemGuard = [&artistElem] (void*) { artistElem = artistElem.nextSiblingElement ("artist"); };
		while (!artistElem.isNull ())
		{
			std::shared_ptr<void> guard (static_cast<void*> (0), elemGuard);

			const auto& name = artistElem.firstChildElement ("name").text ();
			if (name.isEmpty ())
				continue;

			QStringList similarTo;
			auto similarElem = artistElem.firstChildElement ("context").firstChildElement ("artist");
			while (!similarElem.isNull ())
			{
				similarTo << similarElem.firstChildElement ("name").text ();
				similarElem = similarElem.nextSiblingElement ("artist");
			}

			++InfosWaiting_;

			QMap<QString, QString> params;
			params ["artist"] = name;
			AddLanguageParam (params);
			auto infoReply = Request ("artist.getInfo", NAM_, params);

			infoReply->setProperty ("SimilarTo", similarTo);
			connect (infoReply,
					SIGNAL (finished ()),
					this,
					SLOT (handleInfoReplyFinished ()));
			connect (infoReply,
					SIGNAL (error (QNetworkReply::NetworkError)),
					this,
					SLOT (handleInfoReplyError ()));
		}
	}
}
}

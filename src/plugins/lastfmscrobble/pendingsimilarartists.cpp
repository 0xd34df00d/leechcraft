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

#include "pendingsimilarartists.h"
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	PendingSimilarArtists::PendingSimilarArtists (const QString& name,
			int num, QNetworkAccessManager *nam, QObject *parent)
	: BaseSimilarArtists (name, num, parent)
	, NAM_ (nam)
	{
		QMap<QString, QString> params;
		params ["artist"] = name;
		params ["autocorrect"] = "1";
		params ["limit"] = QString::number (num);
		auto reply = Request ("artist.getSimilar", nam, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleReplyError ()));
	}

	void PendingSimilarArtists::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		QDomDocument doc;
		if (!doc.setContent (reply->readAll ()))
		{
			emit ready ();
			return;
		}

		const auto& artistElems = doc.elementsByTagName ("artist");
		if (artistElems.isEmpty ())
		{
			emit ready ();
			return;
		}

		QList<QPair<QString, double>> similar;
		for (int i = 0, size = artistElems.size (); i < size; ++i)
		{
			const auto& elem = artistElems.at (i).toElement ();
			similar << qMakePair (elem.firstChildElement ("name").text (),
						elem.firstChildElement ("match").text ().toDouble ());
		}

		auto begin = similar.begin ();
		auto end = similar.end ();
		const int distance = std::distance (begin, end);
		if (distance > NumGet_)
			std::advance (begin, distance - NumGet_);

		InfosWaiting_ = std::distance (begin, end);

		for (auto i = begin; i != end; ++i)
		{
			QMap<QString, QString> params;
			params ["artist"] = i->first;
			AddLanguageParam (params);
			auto infoReply = Request ("artist.getInfo", NAM_, params);

			infoReply->setProperty ("Similarity", i->second);
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

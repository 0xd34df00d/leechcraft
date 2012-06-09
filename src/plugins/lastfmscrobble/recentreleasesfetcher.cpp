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

#include "recentreleasesfetcher.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtDebug>
#include <QDomDocument>
#include <interfaces/media/irecentreleases.h>
#include "xmlsettingsmanager.h"
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	RecentReleasesFetcher::RecentReleasesFetcher (bool withRecs, int num, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, MaxNum_ (num)
	{
		const auto& user = XmlSettingsManager::Instance ()
				.property ("lastfm.login").toString ();
		QList<QPair<QString, QString>> params;
		params << QPair<QString, QString> ("user", user);
		params << QPair<QString, QString> ("userecs", withRecs ? "1" : "0");
		auto reply = Request ("user.getNewReleases", nam, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleReplyError ()));
	}

	void RecentReleasesFetcher::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		qDebug () << data;
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply";
			return;
		}

		const auto& docElem = doc.documentElement ();
		if (docElem.attribute ("status") != "ok")
		{
			qWarning () << Q_FUNC_INFO
					<< "reply is not ok:"
					<< docElem.attribute ("status");
			return;
		}

		QList<Media::AlbumRelease> releases;

		static auto months = { "Jan", "Feb", "Mar",
				"Apr", "May", "Jun",
				"Jul", "Aug", "Sep",
				"Oct", "Nov", "Dec" };
		auto album = docElem.firstChildElement ("albums").firstChildElement ("album");
		while (!album.isNull ())
		{
			const auto& strs = album.attribute ("releasedate").split (' ', QString::SkipEmptyParts);
			const int day = strs.value (1).toInt ();
			const int month = std::distance (std::begin (months),
						std::find (std::begin (months), std::end (months), strs.value (2))) + 1;
			const int year = strs.value (3).toInt ();

			const QUrl& thumb = GetImage (album, "large");
			const QUrl& full = GetImage (album, "extralarge");

			Media::AlbumRelease release =
			{
				album.firstChildElement ("name").text (),
				album.firstChildElement ("artist").firstChildElement ("name").text (),
				QDateTime (QDate (year, month, day)),
				thumb,
				full,
				QUrl (album.firstChildElement ("url").text ())
			};
			releases << release;

			album = album.nextSiblingElement ("album");
		}

		emit gotRecentReleases (releases);
	}

	void RecentReleasesFetcher::handleReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();
		reply->deleteLater ();
	}
}
}

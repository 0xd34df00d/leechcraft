/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recentreleasesfetcher.h"
#include <algorithm>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtDebug>
#include <QDomDocument>
#include <util/threads/futures.h>
#include <util/network/handlenetworkreply.h>
#include <util/sll/visitor.h>
#include <util/sll/domchildrenrange.h>
#include "xmlsettingsmanager.h"
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	RecentReleasesFetcher::RecentReleasesFetcher (bool withRecs, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	{
		Promise_.reportStarted ();

		const auto& user = XmlSettingsManager::Instance ()
				.property ("lastfm.login").toString ();
		const QList<QPair<QString, QString>> params
		{
			{ "user", user },
			{ "userecs", withRecs ? "1" : "0" }
		};
		Util::HandleReplySeq (Request ("user.getNewReleases", nam, params), this) >>
				Util::Visitor
				{
					[this] (Util::Void) { Util::ReportFutureResult (Promise_, QString { "Unable to send network request." }); },
					[this] (const QByteArray& data) { HandleData (data); }
				}.Finally ([this] { deleteLater (); });
	}

	QFuture<Media::IRecentReleases::Result_t> RecentReleasesFetcher::GetFuture ()
	{
		return Promise_.future ();
	}

	void RecentReleasesFetcher::HandleData (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply";
			Util::ReportFutureResult (Promise_, QString { "Error parsing reply." });
			return;
		}

		const auto& docElem = doc.documentElement ();
		if (docElem.attribute ("status") != "ok")
		{
			qWarning () << Q_FUNC_INFO
					<< "reply is not ok:"
					<< docElem.attribute ("status");
			Util::ReportFutureResult (Promise_, QString { "Error parsing reply." });
			return;
		}

		QList<Media::AlbumRelease> releases;

		const auto months = { "Jan", "Feb", "Mar",
				"Apr", "May", "Jun",
				"Jul", "Aug", "Sep",
				"Oct", "Nov", "Dec" };
		const auto monthsBegin = months.begin ();
		const auto monthsEnd = months.end ();
		for (const auto& album : Util::DomChildren (docElem.firstChildElement ("albums"), "album"))
		{
			const auto& relDate = album.attribute ("releasedate");
			const auto& strs = relDate.splitRef (' ', Qt::SkipEmptyParts);
			const int day = strs.value (1).toInt ();
			const int month = std::distance (monthsBegin,
						std::find (monthsBegin, monthsEnd, strs.value (2))) + 1;
			const int year = strs.value (3).toInt ();

			const QUrl& thumb = GetImage (album, "large");
			const QUrl& full = GetImage (album, "extralarge");

			Media::AlbumRelease release =
			{
				album.firstChildElement ("name").text (),
				album.firstChildElement ("artist").firstChildElement ("name").text (),
				QDate { year, month, day }.startOfDay (),
				thumb,
				full,
				QUrl (album.firstChildElement ("url").text ())
			};
			releases << release;
		}

		Util::ReportFutureResult (Promise_, releases);
	}
}
}

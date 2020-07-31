/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hypedtracksfetcher.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QtDebug>
#include <util/network/handlenetworkreply.h>
#include <util/threads/futures.h>
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	HypedTracksFetcher::HypedTracksFetcher (QNetworkAccessManager *nam, Media::IHypesProvider::HypeType type, QObject *parent)
	: QObject (parent)
	{
		Promise_.reportStarted ();

		QMap<QString, QString> params;
		params ["limit"] = "50";
		const auto& method = type == Media::IHypesProvider::HypeType::NewTracks ?
				"chart.getHypedTracks" :
				"chart.getTopTracks";
		auto reply = Request (method, nam, params);
		Util::HandleReplySeq (reply, this) >>
				Util::Visitor
				{
					[this] (Util::Void) { Util::ReportFutureResult (Promise_, QString { "Unable to issue Last.FM API request." }); },
					[this] (const QByteArray& data) { HandleFinished (data); }
				}.Finally ([this] { deleteLater (); });
	}

	QFuture<Media::IHypesProvider::HypeQueryResult_t> HypedTracksFetcher::GetFuture ()
	{
		return Promise_.future ();
	}

	void HypedTracksFetcher::HandleFinished (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply"
					<< data;
			Util::ReportFutureResult (Promise_, QString { "Unable to parse Last.FM response." });
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
				getText ("playcount").toInt (),
				getText ("listeners").toInt (),
				getText ("duration").toInt (),
				GetImage (trackElem, "medium"),
				GetImage (trackElem, "extralarge"),
				artistElem.firstChildElement ("name").text (),
				artistElem.firstChildElement ("url").text ()
			};

			trackElem = trackElem.nextSiblingElement ("track");
		}

		Util::ReportFutureResult (Promise_, tracks);
	}
}
}

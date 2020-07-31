/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingrecommendedartists.h"
#include <QDomDocument>
#include <QNetworkReply>
#include <QtDebug>
#include <util/sll/util.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/prelude.h>
#include <util/network/handlenetworkreply.h>
#include "authenticator.h"
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	PendingRecommendedArtists::PendingRecommendedArtists (Authenticator *auth,
			QNetworkAccessManager *nam, int num, QObject *obj)
	: BaseSimilarArtists (num, obj)
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
		const ParamsList_t params { { "limit", QString::number (NumGet_) } };
		Util::HandleReplySeq (Request ("user.getRecommendedArtists", NAM_, params), this) >>
				Util::Visitor
				{
					[this] (Util::Void) { ReportError ("Unable to query last.fm."); },
					[this] (const QByteArray& data) { HandleData (data); }
				};
	}

	void PendingRecommendedArtists::HandleData (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse reply";
			ReportError ("Unable to parse service reply.");
			return;
		}

		const auto& recsElem = doc.documentElement ().firstChildElement ("recommendations");
		for (const auto& artistElem : Util::DomChildren (recsElem, "artist"))
		{
			const auto& name = artistElem.firstChildElement ("name").text ();
			if (name.isEmpty ())
				continue;

			const auto& similarTo = Util::Map (Util::DomChildren (artistElem.firstChildElement ("context"), "artist"),
					[] (const auto& elem) { return elem.firstChildElement ("name").text (); });

			++InfosWaiting_;

			QMap<QString, QString> params { { "artist", name } };
			AddLanguageParam (params);
			HandleReply (Request ("artist.getInfo", NAM_, params), {}, similarTo);
		}

		if (!InfosWaiting_)
			ReportError ("No results from last.fm.");
	}
}
}

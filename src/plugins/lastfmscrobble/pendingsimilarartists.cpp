/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingsimilarartists.h"
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <util/network/handlenetworkreply.h>
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	PendingSimilarArtists::PendingSimilarArtists (const QString& name,
			int num, QNetworkAccessManager *nam, QObject *parent)
	: BaseSimilarArtists (num, parent)
	, NAM_ (nam)
	{
		QMap<QString, QString> params
		{
			{ "artist", name },
			{ "autocorrect", "1" },
			{ "limit", QString::number (num) }
		};
		Util::HandleReplySeq (Request ("artist.getSimilar", nam, params), this) >>
				Util::Visitor
				{
					[this] (Util::Void) { ReportError ("Unable to query last.fm."); },
					[this] (const QByteArray& data) { HandleData (data); }
				};
	}

	void PendingSimilarArtists::HandleData (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			ReportError ("Unable to parse last.fm reply.");
			return;
		}

		const auto& artistElems = doc.elementsByTagName ("artist");
		if (artistElems.isEmpty ())
		{
			ReportError ("No results from last.fm.");
			return;
		}

		QList<QPair<QString, double>> similar;
		for (int i = 0, size = artistElems.size (); i < size; ++i)
		{
			const auto& elem = artistElems.at (i).toElement ();
			similar << qMakePair (elem.firstChildElement ("name").text (),
						elem.firstChildElement ("match").text ().toDouble () * 100);
		}

		auto begin = similar.begin ();
		auto end = similar.end ();
		if (const auto distance = end - begin; distance > NumGet_)
			std::advance (begin, distance - NumGet_);

		InfosWaiting_ = std::distance (begin, end);

		for (auto i = begin; i != end; ++i)
		{
			QMap<QString, QString> params { { "artist", i->first } };
			AddLanguageParam (params);
			HandleReply (Request ("artist.getInfo", NAM_, params), i->second, {});
		}
	}
}
}

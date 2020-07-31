/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "basesimilarartists.h"
#include <QNetworkReply>
#include <QDomDocument>
#include <QtDebug>
#include <util/network/handlenetworkreply.h>
#include <util/sll/util.h>
#include "util.h"

namespace LC
{
namespace Lastfmscrobble
{
	BaseSimilarArtists::BaseSimilarArtists (int num, QObject *parent)
	: QObject (parent)
	, NumGet_ (num)
	{
		Promise_.reportStarted ();
	}

	QFuture<Media::SimilarityQueryResult_t> BaseSimilarArtists::GetFuture ()
	{
		return Promise_.future ();
	}

	void BaseSimilarArtists::ReportError (const QString& msg)
	{
		Util::ReportFutureResult (Promise_, msg);
		deleteLater ();
	}

	void BaseSimilarArtists::HandleReply (QNetworkReply *reply,
			const std::optional<int>& similarity,
			const std::optional<QStringList>& similarTo)
	{
		Util::HandleReplySeq (reply, this) >>
				Util::Visitor
				{
					[this] (Util::Void) { DecrementWaiting (); },
					[=] (const QByteArray& data) { HandleInfoReplyFinished (data, similarity, similarTo); }
				};
	}

	void BaseSimilarArtists::DecrementWaiting ()
	{
		--InfosWaiting_;

		if (!InfosWaiting_)
			Util::ReportFutureResult (Promise_, Similar_);
	}

	void BaseSimilarArtists::HandleInfoReplyFinished (const QByteArray& data,
			const std::optional<int>& similarity,
			const std::optional<QStringList>& similarTo)
	{
		const auto decrGuard = Util::MakeScopeGuard ([this] { DecrementWaiting (); });

		QDomDocument doc;
		QString errMsg;
		int errLine = 0, errCol = 0;
		if (!doc.setContent (data, &errMsg, &errLine, &errCol))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse response:"
					<< errMsg
					<< "at"
					<< errLine
					<< ":"
					<< errCol
					<< data;
			return;
		}

		const auto& info = GetArtistInfo (doc.documentElement ().firstChildElement ("artist"));
		Similar_ << Media::SimilarityInfo { info, similarity.value_or (0), similarTo.value_or (QStringList {}) };
	}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "basesimilarartists.h"
#include <QNetworkReply>
#include <QDomDocument>
#include <QtDebug>
#include "util.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	BaseSimilarArtists::BaseSimilarArtists (const QString& name, int num, QObject *parent)
	: QObject (parent)
	, SourceName_ (name)
	, NumGet_ (num)
	, InfosWaiting_ (0)
	{
	}

	QObject* BaseSimilarArtists::GetQObject ()
	{
		return this;
	}

	QString BaseSimilarArtists::GetSourceArtistName () const
	{
		return SourceName_;
	}

	Media::SimilarityInfos_t BaseSimilarArtists::GetSimilar () const
	{
		return Similar_;
	}

	void BaseSimilarArtists::DecrementWaiting ()
	{
		--InfosWaiting_;

		if (!InfosWaiting_)
			emit ready ();
	}

	void BaseSimilarArtists::handleInfoReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const int similarity = reply->property ("Similarity").toInt ();
		const auto& similarTo = reply->property ("SimilarTo").toStringList ();

		const auto& data = reply->readAll ();

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
			DecrementWaiting ();
			return;
		}

		const auto& info = GetArtistInfo (doc.documentElement ().firstChildElement ("artist"));
		Similar_ << Media::SimilarityInfo { info, similarity, similarTo };

		DecrementWaiting ();
	}

	void BaseSimilarArtists::handleInfoReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		DecrementWaiting ();
	}

	void BaseSimilarArtists::handleReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();
		qWarning () << "the query URL was:";
		qWarning () << reply->request ().url ();
		qWarning () << reply->readAll ();

		emit error ();
	}
}
}

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

	QObject* BaseSimilarArtists::GetObject ()
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
		qDebug () << Q_FUNC_INFO << InfosWaiting_ << Similar_.size ();

		if (!InfosWaiting_)
			emit ready ();
	}

	namespace
	{
		Media::ArtistInfo GetAdditional (const QByteArray& raw)
		{
			Media::ArtistInfo result;

			QDomDocument doc;
			if (!doc.setContent (raw))
				return result;

			auto text = [&doc] (const QString& elemName)
			{
				auto items = doc.elementsByTagName (elemName);
				if (items.isEmpty ())
					return QString ();
				auto str = items.at (0).toElement ().text ();
				str.replace ('\r', '\n');
				str.remove ("\n\n");
				str.replace ("&quot;", "\"");
				return str;
			};

			result.ShortDesc_ = text ("summary");
			result.FullDesc_ = text ("content");

			const auto& artist = doc.documentElement ().firstChildElement ("artist");
			result.Image_ = GetImage (artist, "extralarge");
			result.LargeImage_ = GetImage (artist, "mega");

			return result;
		}
	}

	void BaseSimilarArtists::handleInfoReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const int similarity = reply->property ("Similarity").toInt ();
		const auto& similarTo = reply->property ("SimilarTo").toStringList ();

		QDomDocument doc;
		if (!doc.setContent (reply->readAll ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse response";
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

		emit error ();
	}
}
}

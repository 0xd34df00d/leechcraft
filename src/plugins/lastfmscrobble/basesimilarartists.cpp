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
#include <lastfm/Tag>
#include <lastfm/Artist>

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
			return result;
		}
	}

	void BaseSimilarArtists::handleInfoReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const int similarity = reply->property ("Similarity").toInt ();

		const auto& augment = GetAdditional (reply->peek (reply->bytesAvailable ()));

		const auto& artist = lastfm::Artist::getInfo (reply);
		Media::ArtistInfo info =
		{
			artist.name (),
			augment.ShortDesc_,
			augment.FullDesc_,
			artist.imageUrl (lastfm::Large),
			artist.imageUrl (lastfm::ExtraLarge),
			artist.www (),
			Media::TagInfos_t ()
		};
		Similar_ << Media::SimilarityInfo { info, similarity, QStringList () };

		auto tagsReply = artist.getTopTags ();
		tagsReply->setProperty ("Position", Similar_.size () - 1);
		connect (tagsReply,
				SIGNAL (finished ()),
				this,
				SLOT (handleTagsReplyFinished ()));
		connect (tagsReply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleTagsReplyError ()));
	}

	void BaseSimilarArtists::handleInfoReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		DecrementWaiting ();
	}

	void BaseSimilarArtists::handleTagsReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& tags = lastfm::Tag::list (reply).values ();
		Media::TagInfos_t infos;
		std::transform (tags.begin (), tags.end (), std::back_inserter (infos),
				[] (const QString& name) { Media::TagInfo info = { name }; return info; });

		const int pos = reply->property ("Position").toInt ();
		if (Similar_.size () > pos && pos >= 0)
			Similar_ [pos].Artist_.Tags_ = infos;

		DecrementWaiting ();
	}

	void BaseSimilarArtists::handleTagsReplyError ()
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

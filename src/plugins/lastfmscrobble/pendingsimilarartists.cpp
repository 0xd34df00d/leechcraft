/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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
#include <lastfm/Artist>
#include <lastfm/Tag>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	PendingSimilarArtists::PendingSimilarArtists (const QString& name, QObject *parent)
	: QObject (parent)
	, InfosWaiting_ (0)
	{
		auto reply = lastfm::Artist (name).getSimilar ();
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
	}

	QObject* PendingSimilarArtists::GetObject ()
	{
		return this;
	}

	Media::SimilarityInfos_t PendingSimilarArtists::GetSimilar () const
	{
		return Similar_;
	}

	void PendingSimilarArtists::DecrementWaiting ()
	{
		--InfosWaiting_;

		if (!InfosWaiting_)
			emit ready ();
	}

	void PendingSimilarArtists::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& similar = lastfm::Artist::getSimilar (reply);
		if (similar.isEmpty ())
		{
			emit ready ();
			return;
		}

		for (auto i = similar.begin (), end = similar.end (); i != end; ++i)
		{
			auto infoReply = lastfm::Artist (i.value ()).getInfo ();
			infoReply->setProperty ("Similarity", i.key ());
			connect (infoReply,
					SIGNAL (finished ()),
					this,
					SLOT (handleInfoReplyFinished ()));
			connect (infoReply,
					SIGNAL (finished ()),
					this,
					SLOT (handleInfoReplyError ()));
		}
	}

	void PendingSimilarArtists::handleReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		emit error ();
	}

	void PendingSimilarArtists::handleInfoReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const int similarity = reply->property ("Similarity").toInt ();

		const auto& artist = lastfm::Artist::getInfo (reply);
		Media::ArtistInfo info =
		{
			artist.name (),
			artist.imageUrl (lastfm::Large),
			artist.imageUrl (lastfm::ExtraLarge),
			artist.www (),
			Media::TagInfos_t ()
		};
		Similar_ << Media::SimilarityInfo_t (info, similarity);

		auto tagsReply = artist.getTopTags ();
		tagsReply->setProperty ("Position", Similar_.size () - 1);
		connect (tagsReply,
				SIGNAL (finished ()),
				this,
				SLOT (handleTagsReplyFinished ()));
		connect (tagsReply,
				SIGNAL (finished ()),
				this,
				SLOT (handleTagsReplyError ()));
	}

	void PendingSimilarArtists::handleInfoReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		DecrementWaiting ();
	}

	void PendingSimilarArtists::handleTagsReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& tags = lastfm::Tag::list (reply).values ();
		Media::TagInfos_t infos;
		std::transform (tags.begin (), tags.end (), std::back_inserter (infos),
				[] (const QString& name) { Media::TagInfo info = { name }; return info; });

		const int pos = reply->property ("Position").toInt ();
		if (Similar_.size () > pos && pos >= 0)
			Similar_ [pos].first.Tags_ = infos;

		DecrementWaiting ();
	}

	void PendingSimilarArtists::handleTagsReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		DecrementWaiting ();
	}
}
}

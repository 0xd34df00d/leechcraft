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

#include "pendingsimilarartists.h"
#include <QNetworkReply>
#include <lastfm/Artist>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	PendingSimilarArtists::PendingSimilarArtists (const QString& name, int num, QObject *parent)
	: BaseSimilarArtists (name, num, parent)
	{
		auto reply = lastfm::Artist (name).getSimilar ();
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleReplyError ()));
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

		auto begin = similar.begin ();
		auto end = similar.end ();
		const int distance = std::distance (begin, end);
		if (distance > NumGet_)
			std::advance (begin, distance - NumGet_);

		InfosWaiting_ = std::distance (begin, end);

		for (auto i = begin; i != end; ++i)
		{
			auto infoReply = lastfm::Artist (i.value ()).getInfo ();
			infoReply->setProperty ("Similarity", i.key ());
			connect (infoReply,
					SIGNAL (finished ()),
					this,
					SLOT (handleInfoReplyFinished ()));
			connect (infoReply,
					SIGNAL (error (QNetworkReply::NetworkError)),
					this,
					SLOT (handleInfoReplyError ()));
		}
	}
}
}

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

#include "pendingrecommendedartists.h"
#include <lastfm/AuthenticatedUser>
#include <lastfm/Artist>
#include "authenticator.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	PendingRecommendedArtists::PendingRecommendedArtists (Authenticator *auth, int num, QObject *obj)
	: BaseSimilarArtists (QString (), num, obj)
	{
		if (auth)
			request ();
		else
			connect (auth,
					SIGNAL (authenticated ()),
					this,
					SLOT (request ()));
	}

	void PendingRecommendedArtists::request ()
	{
		auto reply = lastfm::AuthenticatedUser ().getRecommendedArtists ();
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleReplyError ()));
	}

	void PendingRecommendedArtists::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& artists = lastfm::Artist::list (reply);
		Q_FOREACH (const auto& artist, artists)
		{
			auto infoReply = artist.getInfo ();
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

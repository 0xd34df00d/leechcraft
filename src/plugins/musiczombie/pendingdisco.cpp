/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "pendingdisco.h"
#include <memory>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QtDebug>
#include "artistlookup.h"

namespace LeechCraft
{
namespace MusicZombie
{
	PendingDisco::PendingDisco (const QString& artist, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	{
		auto idLookup = new ArtistLookup (artist, nam, this);
		connect (idLookup,
				SIGNAL(gotID (QString)),
				this,
				SLOT (handleGotID (QString)));
		connect (idLookup,
				SIGNAL (replyError ()),
				this,
				SLOT (handleIDError ()));
		connect (idLookup,
				SIGNAL (networkError ()),
				this,
				SLOT (handleIDError ()));
	}

	QObject* PendingDisco::GetObject ()
	{
		return this;
	}

	QList<Media::ReleaseInfo> PendingDisco::GetReleases () const
	{
		return Releases_;
	}

	void PendingDisco::handleGotID (const QString& id)
	{
		const auto& urlStr = "http://musicbrainz.org/ws/2/artist/" + id + "?inc=releases";
		auto reply = NAM_->get (QNetworkRequest (QUrl (urlStr)));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleLookupFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleLookupError ()));
	}

	void PendingDisco::handleIDError ()
	{
		emit error (tr ("Error getting artist MBID."));
		deleteLater ();
	}

	void PendingDisco::handleLookupFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse"
					<< data;
			emit error (tr ("Unable to parse MusicBrainz reply."));
			deleteLater ();
		}

		QMap<QString, QMap<QString, Media::ReleaseInfo>> infos;

		auto releaseElem = doc
				.documentElement ()
				.firstChildElement ("artist")
				.firstChildElement ("release-list")
				.firstChildElement ("release");
		while (!releaseElem.isNull ())
		{
			std::shared_ptr<void> guard (static_cast<void*> (0),
					[&releaseElem] (void*)
						{ releaseElem = releaseElem.nextSiblingElement ("release"); });

			auto elemText = [&releaseElem] (const QString& sub)
			{
				return releaseElem.firstChildElement (sub).text ();
			};

			if (elemText ("status") != "Official")
				continue;

			const auto& title = elemText ("title");
			infos [title] [elemText ("country")] =
			{
				title,
				elemText ("date").left (4).toInt (),
				Media::ReleaseInfo::Type::Standard
			};
		}

		for (const auto& key : infos.keys ())
		{
			const auto& countries = infos [key];
			if (countries.contains ("US"))
				Releases_ << countries ["US"];
			else
				Releases_ << countries.values ().first ();
		}

		std::sort (Releases_.begin (), Releases_.end (),
				[] (decltype (Releases_.at (0)) left, decltype (Releases_.at (0)) right)
					{ return left.Year_ < right.Year_; });

		emit ready ();
		deleteLater ();
	}

	void PendingDisco::handleLookupError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< "error lookup stuff up"
				<< reply->errorString ();
		emit error (tr ("Error performing artist lookup: %1.")
					.arg (reply->errorString ()));
	}
}
}

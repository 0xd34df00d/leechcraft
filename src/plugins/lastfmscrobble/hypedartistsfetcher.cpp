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

#include "hypedartistsfetcher.h"
#include <algorithm>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QtDebug>
#include "util.h"
#include "pendingartistbio.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	HypedArtistsFetcher::HypedArtistsFetcher (QNetworkAccessManager *nam, Media::IHypesProvider::HypeType type, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, Type_ (type)
	, InfoCount_ (0)
	{
		QMap<QString, QString> params;
		params ["limit"] = "20";
		auto reply = Request ("chart.getHypedArtists", nam, params);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));
	}

	void HypedArtistsFetcher::DecrementWaiting ()
	{
		if (--InfoCount_)
			return;

		emit gotHypedArtists (Infos_, Type_);
		deleteLater ();
	}

	void HypedArtistsFetcher::handleFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing reply"
					<< data;
			return;
		}

		auto artistElem = doc
				.documentElement ()
				.firstChildElement ("artists")
				.firstChildElement ("artist");
		while (!artistElem.isNull ())
		{
			auto getText = [&artistElem] (const QString& name)
			{
				return artistElem.firstChildElement (name).text ();
			};

			const auto& name = getText ("name");

			Infos_ << Media::HypedArtistInfo
			{
				Media::ArtistInfo
				{
					name,
					QString (),
					QString (),
					GetImage (artistElem, "medium"),
					GetImage (artistElem, "extralarge"),
					getText ("url"),
					Media::TagInfos_t ()
				},
				getText ("percentagechange").toInt ()
			};

			auto pendingBio = new PendingArtistBio (name, NAM_, this);
			connect (pendingBio,
					SIGNAL (ready ()),
					this,
					SLOT (pendingBioReady ()));
			connect (pendingBio,
					SIGNAL (error ()),
					this,
					SLOT (pendingBioError ()));

			artistElem = artistElem.nextSiblingElement ("artist");
		}

		InfoCount_ = Infos_.size ();
		if (!InfoCount_)
			deleteLater ();
	}

	void HypedArtistsFetcher::handleError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		reply->deleteLater ();
		deleteLater ();
	}

	void HypedArtistsFetcher::pendingBioReady ()
	{
		auto pendingBio = qobject_cast<PendingArtistBio*> (sender ());
		pendingBio->deleteLater ();

		const auto& info = pendingBio->GetArtistBio ();
		const auto pos = std::find_if (Infos_.begin (), Infos_.end (),
				[&info] (decltype (Infos_.at (0)) thatInfo) { return thatInfo.Info_.Name_ == info.BasicInfo_.Name_; });
		if (pos != Infos_.end ())
			pos->Info_ = info.BasicInfo_;

		DecrementWaiting ();
	}

	void HypedArtistsFetcher::pendingBioError ()
	{
		sender ()->deleteLater ();

		DecrementWaiting ();
	}
}
}

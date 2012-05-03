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

#include "albumartfetcher.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QDomDocument>
#include <QStringList>
#include <lastfm/Album>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	AlbumArtFetcher::AlbumArtFetcher (const Media::AlbumInfo& albumInfo, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
		lastfm::Album album (lastfm::Artist (albumInfo.Artist_), albumInfo.Album_);
		auto reply = album.getInfo ();
		reply->setProperty ("AlbumInfo", QVariant::fromValue (albumInfo));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
	}

	void AlbumArtFetcher::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& albumInfo = reply->property ("AlbumInfo").value<Media::AlbumInfo> ();
		QDomDocument doc;
		if (!doc.setContent (reply->readAll ()))
		{
			emit gotAlbumArt (albumInfo, QList<QImage> ());
			deleteLater ();
			return;
		}

		const auto& elems = doc.elementsByTagName ("image");
		QStringList sizes;
		sizes << "mega"
				<< "extralarge"
				<< "large"
				<< "medium"
				<< "small"
				<< "";
		while (!sizes.isEmpty ())
		{
			const auto& size = sizes.takeFirst ();
			for (int i = 0; i < elems.size (); ++i)
			{
				const auto& elem = elems.at (i).toElement ();
				if (elem.attribute ("size") != size)
					continue;

				const auto& text = elem.text ();
				if (text.isEmpty ())
					continue;

				QNetworkRequest req (QUrl (elem.text ()));
				req.setPriority (QNetworkRequest::LowPriority);
				auto imageReply = Proxy_->GetNetworkAccessManager ()->get (req);
				imageReply->setProperty ("AlbumInfo", reply->property ("AlbumInfo"));
				connect (imageReply,
						SIGNAL (finished ()),
						this,
						SLOT (handleImageReplyFinished ()));
				return;
			}
		}

		emit gotAlbumArt (albumInfo, QList<QImage> ());
		deleteLater ();
	}

	void AlbumArtFetcher::handleImageReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		deleteLater ();

		auto albumInfo = reply->property ("AlbumInfo").value<Media::AlbumInfo> ();
		QImage image;
		image.loadFromData (reply->readAll ());
		if (!image.isNull ())
			emit gotAlbumArt (albumInfo, QList<QImage> () << image);
	}
}
}

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

#include "util.h"
#include <algorithm>
#include <QCryptographicHash>
#include <QUrl>
#include <QDomElement>
#include <lastfm/ws.h>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	QByteArray MakeCall (QList<QPair<QString, QString>> params)
	{
		std::sort (params.begin (), params.end (),
				[] (decltype (*params.constEnd ()) left, decltype (*params.constEnd ()) right)
					{ return left.first < right.first; });
		auto str = std::accumulate (params.begin (), params.end (), QString (),
				[] (const QString& str, decltype (params.front ()) pair)
					{ return str + pair.first + pair.second; });
		str += lastfm::ws::SharedSecret;
		const auto& sig = QCryptographicHash::hash (str.toUtf8 (), QCryptographicHash::Md5).toHex ();

		params << QPair<QString, QString> ("api_sig", sig);

		QUrl url;
		std::for_each (params.begin (), params.end (),
				[&url] (decltype (params.front ()) pair) { url.addQueryItem (pair.first, pair.second); });
		return url.encodedQuery ();
	}

	QNetworkReply* Request (const QString& method, QNetworkAccessManager *nam, const QMap<QString, QString>& map)
	{
		QList<QPair<QString, QString>> params;
		Q_FOREACH (const auto& key, map.keys ())
			params << qMakePair (key, map [key]);
		return Request (method, nam, params);
	}

	QNetworkReply* Request (const QString& method, QNetworkAccessManager *nam, QList<QPair<QString, QString>> params)
	{
		QNetworkRequest req (QUrl ("http://ws.audioscrobbler.com/2.0/"));
		params << QPair<QString, QString> ("method", method);
		params << QPair<QString, QString> ("api_key", lastfm::ws::ApiKey);
		params << QPair<QString, QString> ("sk", lastfm::ws::SessionKey);

		const auto& data = MakeCall (params);
		req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		return nam->post (req, data);
	}

	Media::ArtistInfo GetArtistInfo (const QDomElement& artist)
	{
		Media::ArtistInfo result;

		auto text = [&artist] (const QString& elemName)
		{
			const auto& items = artist.elementsByTagName (elemName);
			if (items.isEmpty ())
				return QString ();
			auto str = items.at (0).toElement ().text ();
			str.replace ('\r', '\n');
			str.remove ("\n\n");
			str.replace ("&quot;", "\"");
			return str;
		};

		result.Name_ = artist.firstChildElement ("name").text ();
		result.Page_ = artist.firstChildElement ("url").text ();
		result.ShortDesc_ = text ("summary");
		result.FullDesc_ = text ("content");
		result.Image_ = GetImage (artist, "extralarge");
		result.LargeImage_ = GetImage (artist, "mega");

		const auto& tags = artist.elementsByTagName ("tag");
		for (int i = 0; i < tags.size (); ++i)
		{
			const Media::TagInfo tagInfo =
			{
				tags.at (i).firstChildElement ("name").text ()
			};
			result.Tags_.prepend (tagInfo);
		}

		return result;
	}

	QUrl GetImage (const QDomElement& parent, const QString& size)
	{
		auto image = parent.firstChildElement ("image");
		while (!image.isNull ())
		{
			if (image.attribute ("size") == size)
				return image.text ();
			image = image.nextSiblingElement ("image");
		}
		return QUrl ();
	}
}
}

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

#include "util.h"
#include <algorithm>
#include <numeric>
#include <QCryptographicHash>
#include <QUrl>
#include <QDomElement>
#include <QUrlQuery>
#include <ws.h>
#include <util/util.h>
#include <util/sll/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	namespace
	{
		void AppendSig (ParamsList_t& params)
		{
			std::sort (params.begin (), params.end (), Util::ComparingBy (Util::Fst));
			auto str = std::accumulate (params.begin (), params.end (), QString (),
					[] (const QString& str, const auto& pair) { return str + pair.first + pair.second; });
			str += lastfm::ws::SharedSecret;
			const auto& sig = QCryptographicHash::hash (str.toUtf8 (), QCryptographicHash::Md5).toHex ();

			params.append ({ "api_sig", sig });
		}

		QByteArray Params2PostData (ParamsList_t params)
		{
			AppendSig (params);

			QUrlQuery query;
			for (const auto& pair : params)
				query.addQueryItem (pair.first, pair.second);
			return query.toString (QUrl::FullyEncoded).toUtf8 ();
		}

		void AppendParams2Url (ParamsList_t params, QUrl& url)
		{
			AppendSig (params);

			QUrlQuery query;
			for (const auto& pair : params)
				query.addQueryItem (pair.first, pair.second);
			url.setQuery (query);
		}

		QNetworkReply* MakePostRequest (QNetworkAccessManager *nam, const ParamsList_t& params)
		{
			const auto& data = Params2PostData (params);

			QNetworkRequest req { QUrl { "https://ws.audioscrobbler.com/2.0/" } };
			req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());
			req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
			return nam->post (req, data);
		}

		QNetworkReply* MakeGetRequest (QNetworkAccessManager *nam, const ParamsList_t& params)
		{
			QUrl url { "https://ws.audioscrobbler.com/2.0/" };
			AppendParams2Url (params, url);

			return nam->get (QNetworkRequest { url });
		}
	}

	void AddLanguageParam (QMap<QString, QString>& params)
	{
		const auto& ourLang = XmlSettingsManager::Instance ()
				.property ("Language").toString ().trimmed ().left (2);
		params ["lang"] = ourLang.isEmpty () ? Util::GetLanguage () : ourLang;
	}

	QNetworkReply* Request (const QString& method, QNetworkAccessManager *nam, const ParamsMap_t& map)
	{
		ParamsList_t paramsList;
		paramsList.reserve (map.size ());
		const auto& stlized = Util::StlizeCopy<QPair> (map);
		std::copy (stlized.begin (), stlized.end (), std::back_inserter (paramsList));

		return Request (method, nam, std::move (paramsList));
	}

	QNetworkReply* Request (const QString& method, QNetworkAccessManager *nam, ParamsList_t params)
	{
		params.append ({ "method", method });
		params.append ({ "api_key", lastfm::ws::ApiKey });

		if (!lastfm::ws::SessionKey.isEmpty ())
			params.append ({ "sk", lastfm::ws::SessionKey });

		return XmlSettingsManager::Instance ().property ("UseGetRequests").toBool () ?
				MakeGetRequest (nam, params) :
				MakePostRequest (nam, params);
	}

	Media::ArtistInfo GetArtistInfo (const QDomElement& artist)
	{
		Media::ArtistInfo result;

		auto text = [&artist] (const QString& elemName) -> QString
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

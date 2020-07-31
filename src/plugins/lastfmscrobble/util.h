/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QMap>
#include <interfaces/media/audiostructs.h>

class QNetworkAccessManager;
class QNetworkReply;
class QDomElement;
class QUrl;

namespace LC
{
namespace Lastfmscrobble
{
	using ParamsMap_t = QMap<QString, QString>;
	using ParamsList_t = QList<QPair<QString, QString>>;

	void AddLanguageParam (ParamsMap_t& params);

	QNetworkReply* Request (const QString& method, QNetworkAccessManager *nam,
			const ParamsMap_t& params);
	QNetworkReply* Request (const QString& method, QNetworkAccessManager *nam,
			ParamsList_t params = {});

	Media::ArtistInfo GetArtistInfo (const QDomElement& artist);

	QUrl GetImage (const QDomElement& parent, const QString& imageSize);
}
}

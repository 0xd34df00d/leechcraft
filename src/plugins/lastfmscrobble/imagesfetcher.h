/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/media/iartistbiofetcher.h>

class QNetworkAccessManager;
class QNetworkReply;

namespace LC
{
namespace Lastfmscrobble
{
	class ImagesFetcher : public QObject
	{
		Q_OBJECT

		const QString Artist_;
		QNetworkAccessManager * const NAM_;
		QList<Media::ArtistImage> Images_;
	public:
		ImagesFetcher (const QString&, QNetworkAccessManager*, QObject* = nullptr);
	private:
		void HandleError (const QString&);
		void HandleDone ();

		void HandlePageUrl (const QByteArray&);
		void HandleImagesPageFetched (const QByteArray&);
		void HandlePageParsed (const QByteArray&);
	signals:
		void gotImages (const QList<Media::ArtistImage>&);
	};
}
}

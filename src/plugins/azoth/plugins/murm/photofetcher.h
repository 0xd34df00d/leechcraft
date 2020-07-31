/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QFuture>

class QImage;
class QUrl;
class QNetworkAccessManager;
class QNetworkReply;

namespace LC
{
namespace Util
{
	class QueueManager;
}

namespace Azoth
{
namespace Murm
{
	class PhotoFetcher : public QObject
	{
		QNetworkAccessManager * const NAM_;
		Util::QueueManager * const FetchQueue_;

		QHash<QUrl, QFuture<QImage>> Pending_;
	public:
		PhotoFetcher (QNetworkAccessManager*, QObject* = 0);

		QFuture<QImage> GetImage (const QUrl&);
	};
}
}
}

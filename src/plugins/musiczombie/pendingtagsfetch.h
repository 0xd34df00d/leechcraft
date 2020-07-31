/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFutureInterface>
#include <interfaces/media/itagsfetcher.h>

class QNetworkAccessManager;

namespace LC
{
namespace Util
{
	class QueueManager;
}

namespace MusicZombie
{
	class PendingTagsFetch final : public QObject
	{
		Util::QueueManager * const Queue_;
		QNetworkAccessManager * const NAM_;

		QFutureInterface<Media::AudioInfo> Promise_;
	public:
		PendingTagsFetch (Util::QueueManager*, QNetworkAccessManager*, const QString&);

		QFuture<Media::AudioInfo> GetFuture ();
	private:
		void Request (const QByteArray&, int);
		void HandleReplyFinished ();
	};
}
}

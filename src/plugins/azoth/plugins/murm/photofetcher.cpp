/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "photofetcher.h"
#include <QUrl>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFuture>
#include <QFutureInterface>
#include <QtDebug>
#include <util/sll/queuemanager.h>
#include <util/sll/slotclosure.h>
#include <util/sys/paths.h>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	PhotoFetcher::PhotoFetcher (QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, FetchQueue_ (new Util::QueueManager (100, this))
	{
	}

	namespace
	{
		void HandleReplyFinished (QNetworkReply *reply, QFutureInterface<QImage> iface)
		{
			const auto& data = reply->readAll ();

			reply->deleteLater ();

			const auto& image = QImage::fromData (data);
			iface.reportFinished (&image);
		}
	}

	QFuture<QImage> PhotoFetcher::GetImage (const QUrl& url)
	{
		if (Pending_.contains (url))
			return Pending_.value (url);

		QFutureInterface<QImage> iface;

		const auto& future = iface.future ();
		Pending_ [url] = future;
		FetchQueue_->Schedule ([=] () mutable
				{
					iface.reportStarted ();

					const auto& reply = NAM_->get (QNetworkRequest (url));
					new Util::SlotClosure<Util::DeleteLaterPolicy>
					{
						[=]
						{
							Pending_.remove (url);
							HandleReplyFinished (reply, iface);
						},
						reply,
						SIGNAL (finished ()),
						reply
					};
				});
		return future;
	}
}
}
}

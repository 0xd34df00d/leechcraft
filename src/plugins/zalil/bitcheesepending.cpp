/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bitcheesepending.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>

namespace LC
{
namespace Zalil
{
	BitcheesePending::BitcheesePending (const QString& filename, const ICoreProxy_ptr& proxy, QObject *parent)
	: PendingUploadBase { filename, proxy, parent }
	{
		const auto nam = proxy->GetNetworkAccessManager ();

		QNetworkRequest req { QUrl { "https://dump.bitcheese.net/upload-file" } };
		req.setRawHeader ("Referer", "https://dump.bitcheese.net/");

		const auto multipart = MakeStandardMultipart ();
		if (!multipart)
		{
			deleteLater ();
			return;
		}

		const auto reply = nam->post (req, multipart);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));

		connect (reply,
				SIGNAL (uploadProgress (qint64, qint64)),
				this,
				SLOT (handleUploadProgress (qint64, qint64)));
	}

	void BitcheesePending::handleFinished ()
	{
		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		deleteLater ();

		if (reply->error () != QNetworkReply::NoError)
			return;

		auto location = reply->header (QNetworkRequest::LocationHeader).toString ();
		location.chop (qstrlen ("/preview"));
		emit fileUploaded (Filename_, QUrl { location });
	}
}
}

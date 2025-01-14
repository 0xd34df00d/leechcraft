/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Slava Barinov <rayslava@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pasteorgruservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>

namespace LC::Azoth::Autopaste
{
	void PasteOrgRuService::Paste (const PasteParams& params)
	{
		const QByteArray& data = "type=1&code=" + params.Text_.toUtf8 ().toPercentEncoding ();

		QNetworkRequest req (QString ("http://paste.org.ru/?"));
		req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

		InitReply (params.NAM_->post (req, data));
	}

	void PasteOrgRuService::HandleMetadata (QNetworkReply *reply)
	{
		const auto code = reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt ();
		if (code >= 400)
		{
			qWarning () << Q_FUNC_INFO
					<< "bad HTTP status code:"
					<< code;
			HandleError (QNetworkReply::ProtocolFailure, reply);
			return;
		}

		const QString refreshRaw { reply->rawHeader ("Refresh") };
		const auto& path = refreshRaw.section (";URL=", 1);
		if (path.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unexpected `Refresh` header:"
					<< refreshRaw;
			HandleError (QNetworkReply::ProtocolFailure, reply);
			return;
		}

		FeedURL ("http://paste.org.ru" + path);

		deleteLater ();
	}
}

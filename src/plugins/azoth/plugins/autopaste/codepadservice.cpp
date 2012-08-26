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

#include "codepadservice.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	CodepadService::CodepadService (QObject *entry, QObject *parent)
	: PasteServiceBase (entry, parent)
	{
	}

	void CodepadService::Paste (const PasteParams& params)
	{
		QNetworkRequest req (QUrl ("http://codepad.org"));
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		req.setRawHeader ("Referer", "http://codepad.org");

		QByteArray data = "lang=Plain+Text&code=";
		data += params.Text_.toUtf8 ().toPercentEncoding ();
		data += "&private=True&submit=Submit";

		req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());

		InitReply (params.NAM_->post (req, data));
	}

	void CodepadService::handleMetadata ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a QNetworkReply:"
					<< sender ();
			return;
		}

		FeedURL (reply->header (QNetworkRequest::LocationHeader).toString ());
	}
}
}
}

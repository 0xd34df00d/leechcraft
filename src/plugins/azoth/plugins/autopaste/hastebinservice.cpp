/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hastebinservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <util/sll/parsejson.h>

namespace LC::Azoth::Autopaste
{
	void HastebinService::Paste (const PasteParams& params)
	{
		QNetworkRequest req (QString ("https://hastebin.com/documents"));
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		QByteArray data = params.Text_.toUtf8 ();

		InitReply (params.NAM_->post (req, data));
	}

	void HastebinService::HandleFinished (QNetworkReply *reply)
	{
		const auto& var = Util::ParseJson (reply->readAll (), Q_FUNC_INFO);
		if (var.isNull ())
		{
			HandleError (QNetworkReply::ProtocolFailure, reply);
			return;
		}

		QUrl url ("https://hastebin.com/");
		url.setPath ("/" + var.toMap () ["key"].toString ());
		FeedURL (url.toString ());
	}
}

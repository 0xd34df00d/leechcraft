/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "codepadservice.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>

namespace LC::Azoth::Autopaste
{
	void CodepadService::Paste (const PasteParams& params)
	{
		QNetworkRequest req (QUrl ("http://codepad.org"));
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		req.setRawHeader ("Referer", "http://codepad.org");

		QByteArray highlight = "Plain+Text";
		bool run = false;
		switch (params.High_)
		{
		case Highlight::CPP:
			highlight = "C%2B%2B";
			run = true;
			break;
		case Highlight::C:
			highlight = "C";
			run = true;
			break;
		case Highlight::XML:
			break;
		case Highlight::Haskell:
			highlight = "Haskell";
			run = true;
			break;
		case Highlight::Java:
			highlight = "Java";
			run = true;
			break;
		case Highlight::Python:
			highlight = "Python";
			run = true;
			break;
		case Highlight::Shell:
			highlight = "Shell";
			run = true;
			break;
		case Highlight::None:
			highlight = "Plain+Text";
			break;
		}

		QByteArray data = "lang=" + highlight + "&code=";
		data += params.Text_.toUtf8 ().toPercentEncoding ();
		data += "&private=True&submit=Submit";
		if (run)
			data += "&run=True";

		req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());
		req.setAttribute (QNetworkRequest::FollowRedirectsAttribute, true);

		InitReply (params.NAM_->post (req, data));
	}
}

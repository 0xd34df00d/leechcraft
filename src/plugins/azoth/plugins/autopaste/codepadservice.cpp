/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
		case Highlight::CPP0x:
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

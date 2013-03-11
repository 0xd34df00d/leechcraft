/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

		InitReply (params.NAM_->post (req, data));
	}
}
}
}

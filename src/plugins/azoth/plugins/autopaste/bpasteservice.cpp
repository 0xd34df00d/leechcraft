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

#include "bpasteservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	BPasteService::BPasteService (QObject* entry, QObject *parent)
	: PasteServiceBase (entry, parent)
	{
	}

	void BPasteService::Paste (const PasteParams& params)
	{
		QNetworkRequest req (QUrl ("http://bpaste.net"));
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		req.setRawHeader ("Referer", "http://bpaste.net");

		QByteArray highlight;
		switch (params.High_)
		{
		case Highlight::CPP:
		case Highlight::CPP0x:
			highlight = "cpp";
			break;
		case Highlight::C:
			highlight = "c";
			break;
		case Highlight::XML:
			highlight = "xml";
			break;
		case Highlight::Haskell:
			highlight = "haskell";
			break;
		case Highlight::Java:
			highlight = "java";
			break;
		case Highlight::Python:
			highlight = "python";
			break;
		case Highlight::None:
			break;
		}

		QByteArray data = "language=" + highlight + "&code=";
		data += params.Text_.toUtf8 ().toPercentEncoding ();
		data += "&private=on&webpage=";

		req.setHeader (QNetworkRequest::ContentLengthHeader, data.size ());

		InitReply (params.NAM_->post (req, data));
	}
}
}
}

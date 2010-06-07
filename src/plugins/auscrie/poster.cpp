/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "poster.h"
#include <QNetworkReply>
#include <QUrl>
#include <QtDebug>
#include "requestbuilder.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Auscrie
		{
			Poster::Poster (const QByteArray& data, const QString& format,
					QNetworkAccessManager *am, QObject *parent)
			: QObject (parent)
			{
				QUrl url ("http://imagebin.ca/upload.php");

				RequestBuilder builder;
				builder.AddPair ("t", "file");

				QString name = QString ("screenshot.%1").arg (format.toLower ());
				builder.AddPair ("name", name);
				builder.AddPair ("tags", "leechcraft");
				builder.AddPair ("adult", "f");
				builder.AddFile (format, "f", data);

				QByteArray formed = builder.Build ();

				QNetworkRequest request (url);
				request.setHeader (QNetworkRequest::ContentTypeHeader,
						QString ("multipart/form-data; boundary=" + builder.GetBoundary ()));
				request.setHeader (QNetworkRequest::ContentLengthHeader,
						QString::number (formed.size ()));
				QNetworkReply *reply = am->post (request, formed);

				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleFinished ()));
				connect (reply,
						SIGNAL (error (QNetworkReply::NetworkError)),
						this,
						SLOT (handleError ()));
			}

			void Poster::handleFinished ()
			{
				emit finished (qobject_cast<QNetworkReply*> (sender ()));
			}

			void Poster::handleError ()
			{
				emit error (qobject_cast<QNetworkReply*> (sender ()));
			}
		};
	};
};


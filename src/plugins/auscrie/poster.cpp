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
#include <QUuid>
#include <QtDebug>

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
				QString rnd = QUuid::createUuid ().toString ();
				rnd = rnd.mid (1, rnd.size () - 2);
				rnd += rnd;
				rnd = rnd.left (55);

				Boundary_ = "----------";
				Boundary_ += rnd;

				QUrl url ("http://imagebin.ca/upload.php");
				QByteArray formed = AddPair ("t", "file");
				//url.addQueryItem ("t", "file");

				QString name = QString ("screenshot.%1").arg (format.toLower ());
				formed += AddPair ("name", name);
				//url.addQueryItem ("name", name);

				formed += AddPair ("tags", "leechcraft");
				//url.addQueryItem ("tags", "leechcraft");

				formed += AddPair ("adult", "f");
				//url.addQueryItem ("adult", "f");

				formed += AddFile (format, "f", data);

				formed += "--";
				formed += Boundary_;
				formed += "--";

				QNetworkRequest request (url);
				request.setHeader (QNetworkRequest::ContentTypeHeader,
						QString ("multipart/form-data; boundary=" + Boundary_));
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

			QByteArray Poster::AddPair (const QString& name, const QString& value)
			{
				QByteArray str;

				str += "--";
				str += Boundary_;
				str += "\r\n";
				str += "Content-Disposition: form-data; name=\"";
				str += name.toAscii();
				str += "\"";
				str += "\r\n\r\n";
				str += value.toUtf8();
				str += "\r\n";

				return str;
			}

			QByteArray Poster::AddFile (const QString& format,
					const QString& name, const QByteArray& imageData)
			{
				QByteArray str;

				str += "--";
				str += Boundary_;
				str += "\r\n";
				str += "Content-Disposition: form-data; name=\"";
				str += name.toAscii ();
				str += "\"; ";
				str += "filename=\"";
				str += QString ("screenshot.%1")
					.arg (format.toLower ())
					.toAscii ();
				str += "\"";
				str += "\r\n";
				str += "Content-Type: ";
				if (format.toLower () == "jpg")
					str += "image/jpeg";
				else
					str += "image/png";
				str += "\r\n\r\n";

				str += imageData;
				str += "\r\n";

				return str;
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


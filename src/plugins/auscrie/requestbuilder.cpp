/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "requestbuilder.h"
#include <QUuid>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Auscrie
		{
			RequestBuilder::RequestBuilder ()
			{
				QString rnd = QUuid::createUuid ().toString ();
				rnd = rnd.mid (1, rnd.size () - 2);
				rnd += rnd;
				rnd = rnd.left (55);

				Boundary_ = "----------";
				Boundary_ += rnd;
			}

			void RequestBuilder::AddPair (const QString& name, const QString& value)
			{
				Result_ += "--";
				Result_ += Boundary_;
				Result_ += "\r\n";
				Result_ += "Content-Disposition: form-data; name=\"";
				Result_ += name.toAscii();
				Result_ += "\"";
				Result_ += "\r\n\r\n";
				Result_ += value.toUtf8();
				Result_ += "\r\n";
			}

			void RequestBuilder::AddFile (const QString& format,
					const QString& name, const QByteArray& imageData)
			{
				Result_ += "--";
				Result_ += Boundary_;
				Result_ += "\r\n";
				Result_ += "Content-Disposition: form-data; name=\"";
				Result_ += name.toAscii ();
				Result_ += "\"; ";
				Result_ += "filename=\"";
				Result_ += QString ("screenshot.%1")
					.arg (format.toLower ())
					.toAscii ();
				Result_ += "\"";
				Result_ += "\r\n";
				Result_ += "Content-Type: ";
				if (format.toLower () == "jpg")
					Result_ += "image/jpeg";
				else
					Result_ += "image/png";
				Result_ += "\r\n\r\n";

				Result_ += imageData;
				Result_ += "\r\n";
			}

			QByteArray RequestBuilder::Build ()
			{
				QByteArray formed = Result_;

				formed += "--";
				formed += Boundary_;
				formed += "--";

				return formed;
			}

			QString RequestBuilder::GetBoundary () const
			{
				return Boundary_;
			}
		};
	};
};

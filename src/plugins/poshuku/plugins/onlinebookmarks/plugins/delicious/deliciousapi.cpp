
/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "deliciousapi.h"
#include <QtDebug>
#include <QStringList>
#include <QDateTime>
#include <QXmlStreamReader>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	DeliciousApi::DeliciousApi ()
	{
	}

	QString DeliciousApi::GetAuthUrl (bool) const
	{
		return "https://%1:%2@api.del.icio.us/v1/posts/update";
	}

	QString DeliciousApi::GetUploadUrl (bool) const
	{
		return "https://%1:%2@api.del.icio.us/v1/posts/add?";
	}

	QByteArray DeliciousApi::GetUploadPayload (const QVariant& bookmark)
	{
		QVariantMap map = bookmark.toMap ();

		return QString ("&url=%1&description=%2&tags=%3")
				.arg (map ["URL"].toString (),
						map ["Title"].toString (),
						map ["Tags"].toString ().split (',').join (" ")).toUtf8 ();
	}

	QString DeliciousApi::GetDownloadUrl (bool) const
	{
		return "https://%1:%2@api.del.icio.us/v1/posts/all?";
	}

	QByteArray DeliciousApi::GetDownloadPayload (const QDateTime& from)
	{
		if (from.isNull ())
			return QString ("&meta=yes").toUtf8 ();
		else
			return QString ("&fromdt=%1&meta=yes").arg (from.toString ("yyyy-MM-ddThh:mm:ssZ")).toUtf8 ();
	}

	QVariantList DeliciousApi::ParseDownloadReply (const QByteArray& content)
	{
		QVariantList list;
		QVariantMap record;
		QXmlStreamReader xml (content);
		while (!xml.atEnd () &&
				!xml.hasError ())
		{
			QXmlStreamReader::TokenType token = xml.readNext ();
			if (token == QXmlStreamReader::StartDocument)
				continue;
			if (token == QXmlStreamReader::StartElement)
				if(xml.name() == "post")
				{
					record ["URL"] = xml.attributes ().value ("href").toString ();
					record ["Title"] = xml.attributes ().value ("description").toString ();;
					record ["Tags"] = xml.attributes ().value ("tag").toString ();
					list << record;
				}
		}

		return list;
	}

	bool DeliciousApi::ParseAuthReply (const QByteArray& content)
	{
		QXmlStreamReader reply (content);
		while (!reply.atEnd ())
		{
			reply.readNext();
			if (reply.name () == "update")
				if (reply.attributes ().hasAttribute ("time"))
					return true;
		}
		if (reply.hasError ())
			return false;

		return false;
	}

	bool DeliciousApi::ParseUploadReply (const QByteArray& content)
	{
		QXmlStreamReader reply (content);
		while (!reply.atEnd ())
		{
			reply.readNext();
			if (reply.name () == "result")
			{
				if (reply.attributes ().hasAttribute ("code") &&
						reply.attributes ().value ("code") == "done")
					return true;
			}
		}
		if (reply.hasError ())
			return false;

		return false;
	}

}
}
}
}


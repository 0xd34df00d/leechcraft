/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "deliciousapi.h"
#include <QtDebug>
#include <QDateTime>
#include <QXmlStreamReader>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	QString DeliciousApi::GetAuthUrl (const DeliciousAccount::AuthType& at) const
	{
		if (at == DeliciousAccount::ATHttpAuth)
			return "https://%1:%2@api.del.icio.us/v1/posts/update";

		qWarning () << Q_FUNC_INFO << "unknown auth type" << at;
		return QString ();
	}

	QString DeliciousApi::GetUploadUrl (const DeliciousAccount::AuthType& at) const
	{
		if (at == DeliciousAccount::ATHttpAuth)
			return "https://%1:%2@api.del.icio.us/v1/posts/add?";

		qWarning () << Q_FUNC_INFO << "unknown auth type" << at;
		return QString ();
	}

	QByteArray DeliciousApi::GetUploadPayload (const QVariant& bookmark)
	{
		QVariantMap map = bookmark.toMap ();

		return QString ("&url=%1&description=%2&tags=%3")
				.arg (map ["URL"].toString (),
						map ["Title"].toString (),
						map ["Tags"].toString ().split (',').join (" ")).toUtf8 ();
	}

	QString DeliciousApi::GetDownloadUrl (const DeliciousAccount::AuthType& at) const
	{
		if (at == DeliciousAccount::ATHttpAuth)
			return "https://%1:%2@api.del.icio.us/v1/posts/all?";

		qWarning () << Q_FUNC_INFO << "unknown auth type" << at;
		return QString ();
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

			if (token == QXmlStreamReader::StartElement &&
					xml.name() == "post")
			{
				record ["URL"] = xml.attributes ().value ("href").toString ();
				record ["Title"] = xml.attributes ().value ("description").toString ();;
				record ["Tags"] = xml.attributes ().value ("tag").toString ();
				list << record;
			}
		}

		if (xml.hasError ())
			qWarning () << Q_FUNC_INFO
					<< "Parsing finished with error"
					<< xml.errorString ();

		return list;
	}

	bool DeliciousApi::ParseAuthReply (const QByteArray& content)
	{
		QXmlStreamReader xml (content);
		while (!xml.atEnd () &&
				!xml.hasError ())
		{
			QXmlStreamReader::TokenType token = xml.readNext ();

			if (token == QXmlStreamReader::StartDocument)
				continue;

			if (token == QXmlStreamReader::StartElement &&
					xml.name() == "update" &&
					xml.attributes ().hasAttribute ("time"))
				return true;
		}

		qWarning () << Q_FUNC_INFO
				<< "Parsing finished with error"
				<< xml.errorString ();

		return false;
	}

	bool DeliciousApi::ParseUploadReply (const QByteArray& content)
	{
		QXmlStreamReader xml (content);
		while (!xml.atEnd () &&
				!xml.hasError ())
		{
			QXmlStreamReader::TokenType token = xml.readNext ();

			if (token == QXmlStreamReader::StartDocument)
				continue;

			if (token == QXmlStreamReader::StartElement &&
					xml.name() == "result" &&
					xml.attributes ().hasAttribute ("code") &&
					xml.attributes ().value ("code") == "done")
				return true;
		}

		qWarning () << Q_FUNC_INFO
				<< "Parsing finished with error"
				<< xml.errorString ();

		return false;
	}

}
}
}
}


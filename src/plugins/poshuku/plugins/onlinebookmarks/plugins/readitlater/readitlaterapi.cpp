/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "readitlaterapi.h"
#include <QtDebug>
#include <QStringList>
#include <QDateTime>
#include <util/sll/parsejson.h>
#include <util/sll/serializejson.h>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	namespace
	{
		const QString ApiKey { "0l7A6m89daNpif742cpM7fRJe9Tcxd49" };
	}

	QString ReadItLaterApi::GetAuthUrl () const
	{
		return "https://readitlaterlist.com/v2/auth?";
	}

	QByteArray ReadItLaterApi::GetAuthPayload (const QString& login, const QString& pass)
	{
		return QString ("username=%1&password=%2&apikey=%3")
				.arg (login, pass, ApiKey).toUtf8 ();
	}

	QString ReadItLaterApi::GetRegisterUrl () const
	{
		return "https://readitlaterlist.com/v2/signup?";
	}

	QByteArray ReadItLaterApi::GetRegisterPayload (const QString& login,
			const QString& pass)
	{
		return GetAuthPayload (login, pass);
	}

	QString ReadItLaterApi::GetUploadUrl () const
	{
		return "https://readitlaterlist.com/v2/send?";
	}

	QByteArray ReadItLaterApi::GetUploadPayload (const QString& login,
			const QString& password, const QVariantList& bookmarks)
	{
		QVariantMap exportBookmarks, exportTags;
		for (const auto& recordVar : bookmarks)
		{
			const auto& record = recordVar.toMap ();

			const auto& url = record ["URL"];

			const auto& tags = record ["Tags"].toString ();
			if (!tags.isEmpty ())
				exportTags.insert (QString::number (exportTags.size ()),
						QVariantMap
						{
							{ "url", url },
							{ "tags", tags }
						});

			exportBookmarks.insert (QString::number (exportBookmarks.size ()),
					QVariantMap
					{
						{ "url", url },
						{ "title", record ["Title"] }
					});
		}

		if (exportBookmarks.isEmpty ())
			return QByteArray ();

		const auto& jsonBookmarks = Util::SerializeJson (exportBookmarks);
		const auto& jsonTags = Util::SerializeJson (exportTags);

		QString res = QString ("username=%1&password=%2&apikey=%3&new=%4&update_tags=%5")
				.arg (login,
					password,
					ApiKey,
					QString::fromUtf8 (jsonBookmarks.constData ()),
					QString::fromUtf8 (jsonTags.constData ()));
		return res.toUtf8 ();
	}

	QString ReadItLaterApi::GetDownloadUrl () const
	{
		return "https://readitlaterlist.com/v2/get?";
	}

	QByteArray ReadItLaterApi::GetDownloadPayload (const QString& login,
			const QString& password, const QDateTime& from)
	{
		return QString ("username=%1&password=%2&apikey=%3&since=%4&tags=1")
				.arg (login,
					password,
					ApiKey,
					!from.isNull () ? QString::number (from.toSecsSinceEpoch ()) : "").toUtf8 ();
	}

	QVariantList ReadItLaterApi::GetDownloadedBookmarks (const QByteArray& content)
	{
		const auto& resultVar = Util::ParseJson (content, Q_FUNC_INFO);
		if (resultVar.isNull ())
			return {};

		const auto& result = resultVar.toMap ();
		const QVariantMap& nestedMap = result ["list"].toMap ();

		QVariantList bookmarks;
		for (const auto& var : nestedMap)
		{
			const auto& map = var.toMap ();
			bookmarks.push_back (QVariantMap
					{
						{ "Tags", map ["tags"] },
						{ "Title", map ["title"] },
						{ "URL", map ["url"] }
					});
		}

		return bookmarks;
	}

}
}
}
}


/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "readitlaterapi.h"
#include <QtDebug>
#include <QStringList>
#include <QDateTime>
#include <util/sll/parsejson.h>
#include <util/sll/serializejson.h>

namespace LeechCraft
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
					!from.isNull () ? QString::number (from.toTime_t ()) : "").toUtf8 ();
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


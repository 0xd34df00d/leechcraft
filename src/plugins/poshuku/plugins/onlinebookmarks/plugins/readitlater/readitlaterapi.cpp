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

#include "readitlaterapi.h"
#include <QtDebug>
#include <QStringList>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <QDateTime>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	ReadItLaterApi::ReadItLaterApi ()
	: ApiKey_ ("0l7A6m89daNpif742cpM7fRJe9Tcxd49")
	{
	}

	QString ReadItLaterApi::GetAuthUrl () const
	{
		return "https://readitlaterlist.com/v2/auth?";
	}

	QByteArray ReadItLaterApi::GetAuthPayload (const QString& login, const QString& pass)
	{
		return QString ("username=%1&password=%2&apikey=%3")
				.arg (login, pass, ApiKey_).toUtf8 ();
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
		int i = 0;
		int j = 0;
		Q_FOREACH (const QVariant& record, bookmarks)
		{
			QVariantMap bookmark, tags;
			bookmark.insert ("url", record.toMap () ["URL"].toString ());
			bookmark.insert ("title", record.toMap () ["Title"].toString ());

			if (!(record.toMap () ["Tags"].toStringList ().isEmpty ()))
			{
				tags.insert ("url", record.toMap () ["URL"].toString ());
				tags.insert ("tags", record.toMap () ["Tags"].toString ());
				exportTags.insert (QString::number (j++), tags);
			}
			exportBookmarks.insert(QString::number (i++), bookmark);
		}

		if (exportBookmarks.isEmpty ())
			return QByteArray ();

		QJson::Serializer serializer;
		QByteArray jsonBookmarks = serializer.serialize (exportBookmarks);
		QByteArray jsonTags = serializer.serialize (exportTags);

		QString res = QString ("username=%1&password=%2&apikey=%3&new=%4&update_tags=%5")
				.arg (login,
					password,
					ApiKey_,
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
					ApiKey_,
					!from.isNull () ? QString::number (from.toTime_t ()) : "").toUtf8 ();
	}

	QVariantList ReadItLaterApi::GetDownloadedBookmarks (const QByteArray& content)
	{
		QJson::Parser parser;
		bool ok;

		const QVariantMap& result = parser.parse (content, &ok).toMap ();

		if (!ok)
			return QVariantList ();

		const QVariantMap& nestedMap = result ["list"].toMap ();

		QVariantList bookmarks;
		Q_FOREACH (const QVariant& var, nestedMap)
		{
			QVariantMap record;
			QVariantMap map = var.toMap ();

			record ["Tags"] = map ["tags"].toStringList ();
			record ["Title"] = map ["title"].toString ();
			record ["URL"] = map ["url"].toString ();

			bookmarks.push_back (record);
		}

		return bookmarks;
	}

}
}
}
}


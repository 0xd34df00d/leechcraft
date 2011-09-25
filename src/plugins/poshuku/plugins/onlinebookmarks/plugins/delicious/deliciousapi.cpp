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
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <QDateTime>

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

	QString DeliciousApi::GetAuthUrl () const
	{
		return QString ();
	}

	QByteArray DeliciousApi::GetAuthPayload (const QString&, const QString&)
	{
		return QByteArray ();
	}

	QString DeliciousApi::GetUploadUrl () const
	{
		return QString ();
	}

	QByteArray DeliciousApi::GetUploadPayload (const QString&,
			const QString&, const QVariantList&)
	{
		return QByteArray ();
	}

	QString DeliciousApi::GetDownloadUrl () const
	{
		return QString ();
	}

	QByteArray DeliciousApi::GetDownloadPayload (const QString& login,
			const QString& password, const QDateTime& from)
	{
		return QByteArray ();
	}

	QVariantList DeliciousApi::GetDownloadedBookmarks (const QByteArray&)
	{
		return QVariantList ();
	}

}
}
}
}


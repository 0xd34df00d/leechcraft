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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSAPI_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSAPI_H

#include <QObject>
#include <QVariant>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	class DeliciousApi : public QObject
	{
		Q_OBJECT
	public:
		DeliciousApi ();
		QString GetAuthUrl (bool oauth = false) const;
		QString GetUploadUrl (bool oauth = false) const;
		QByteArray GetUploadPayload (const QVariant&);
		QString GetDownloadUrl (bool oauth = false) const;
		QByteArray GetDownloadPayload (const QDateTime&);
		QVariantList ParseDownloadReply (const QByteArray&);
		bool ParseAuthReply (const QByteArray&);
		bool ParseUploadReply (const QByteArray&);
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_DELICIOUS_DELICIOUSAPI_H

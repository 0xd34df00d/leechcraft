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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAPI_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAPI_H

#include <QObject>
#include <QUrl>
#include <QVariant>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	class ReadItLaterApi : public QObject
	{
		Q_OBJECT

		const QString ApiKey_;
	public:
		ReadItLaterApi ();
		QString GetAuthUrl () const;
		QByteArray GetAuthPayload (const QString&, const QString&);
		QString GetRegisterUrl () const;
		QByteArray GetRegisterPayload (const QString&, const QString&);
		QString GetUploadUrl () const;
		QByteArray GetUploadPayload (const QString&,
				const QString&, const QVariantList&);
		QString GetDownloadUrl () const;
		QByteArray GetDownloadPayload (const QString&,
				const QString&, const QDateTime&);
		QVariantList GetDownloadedBookmarks (const QByteArray&);
		
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERAPI_H

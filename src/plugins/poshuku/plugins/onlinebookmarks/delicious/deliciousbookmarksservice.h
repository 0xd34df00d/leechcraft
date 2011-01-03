/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_DELICIOUS_DELICIOUSBOOKMARKSSERVICE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_DELICIOUS_DELICIOUSBOOKMARKSSERVICE_H

#include <QIcon>
#include <QUrl>
#include <QNetworkAccessManager>
#include "abstractbookmarksservice.h"

class QNetworkReply;

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace OnlineBookmarks
{
	class DeliciousBookmarksService : public AbstractBookmarksService
	{
		Q_OBJECT

		QNetworkAccessManager Manager_;
		QNetworkReply *Reply_;
		QUrl ApiUrl_;
		QByteArray RequestString_;
	public:
		DeliciousBookmarksService (QWidget* = 0);
		QString GetName () const;
		QIcon GetIcon () const;
		void CheckValidAccountData (const QString&, const QString&);		
	public slots:
		void getReplyFinished ();
		void readyReadReply ();
	signals:
		void gotValidReply (bool);
	};
}
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_DELICIOUSBOOKMARKSSERVICE_H


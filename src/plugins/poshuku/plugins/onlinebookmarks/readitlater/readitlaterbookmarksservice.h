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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_READITLATER_READITLATERBOOKMARKSSERVICE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_READITLATER_READITLATERBOOKMARKSSERVICE_H

#include <QIcon>
#include <QUrl>
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
	class ReadItLaterBookmarksService : public AbstractBookmarksService
	{
		Q_OBJECT
		
		enum ConnectionType
		{
			Auth_ = 0x01,
			Download_ = 0x02,
			Upload_ = 0x03,
			Sync_ = 0x04
		};
		
		ConnectionType Type_;
		QNetworkReply *Reply_;
		QUrl ApiUrl_;
		QByteArray RequestString_;
	public:
		ReadItLaterBookmarksService (QWidget* = 0);
		QString GetName () const;
		QIcon GetIcon () const;
		void CheckValidAccountData (const QString&, const QString&);
		void DownloadBookmarks (const QStringList&, const QDateTime&);
		void UploadBookmarks (const QStringList&, const QList<QVariant>&);
	public slots:
		void getReplyFinished ();
		void readyReadReply ();
	private:
		void FetchBookmarks (const QString&, const QString&, int);
		void SendBookmarks (const QString&, const QString&, const QList<QVariant>&);
		void ParseDownloadReply (const QByteArray&);
		void ParseUploadReply (bool);
	signals:
		void gotValidReply (bool);
		void gotDownloadReply (const QList<QVariant>&, const QUrl&);
		void gotUploadReply (bool);
	};
}
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_READITLATERBOOKMARKSSERVICE_H


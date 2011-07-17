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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ABSTRACTBOOKMARKSSERVICE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_ABSTRACTBOOKMARKSSERVICE_H

#include <QObject>
#include <QIcon>
#include <QUrl>
#include <QDateEdit>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class AbstractBookmarksService : public QObject
	{
		Q_OBJECT
		
	public:
		AbstractBookmarksService (QObject *parent = 0) : QObject (parent) {}

		virtual QString GetName () const = 0;
		virtual QIcon GetIcon () const = 0;
		virtual void CheckValidAccountData (const QString&, const QString&) = 0;
		virtual void DownloadBookmarks (const QStringList&, const QDateTime&) = 0;
		virtual void UploadBookmarks (const QStringList&, const QList<QVariant>&) = 0;
	public slots:
		virtual void getReplyFinished () = 0;
		virtual void readyReadReply () = 0;
	private:
		virtual void ParseDownloadReply (const QByteArray&) = 0;
		virtual void ParseUploadReply (bool) = 0;
	signals:
		void gotValidReply (bool);
		void gotParseError (const QString&);
		void gotDownloadReply (const QList<QVariant>&, const QUrl&);
		void gotUploadReply (bool);
	};
}
}
}

#endif

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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERACCOUNT_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERACCOUNT_H

#include <QObject>
#include <QDateTime>
#include <interfaces/structures.h>
#include <interfaces/iaccount.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
class ReadItLaterAccount : public QObject
							, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Poshuku::OnlineBookmarks::IAccount)

		QString Login_;
		QString Password_;

		QObject *ParentService_;

		bool IsSyncing_;

		bool IsQuickUpload_;

		QDateTime LastUpload_;
		QDateTime LastDownload_;

		QVariantList DownloadedBookmarks_;
	public:
		ReadItLaterAccount (const QString&, QObject* = 0);

		QObject* GetObject ();
		QObject* GetParentService () const;
		QByteArray GetAccountID () const;
		QString GetLogin () const;

		QString GetPassword () const;
		void SetPassword (const QString&);

		bool IsSyncing () const;
		void SetSyncing (bool);

		bool IsQuickUpload () const;
		void SetQuickUpload (bool);

		QDateTime GetLastDownloadDateTime () const;
		QDateTime GetLastUploadDateTime () const;
		void SetLastUploadDateTime (const QDateTime&);
		void SetLastDownloadDateTime (const QDateTime&);

		void AppendDownloadedBookmarks (const QVariantList&);
		QVariantList GetBookmarksDiff (const QVariantList&);
		QByteArray Serialize () const ;
		static ReadItLaterAccount* Deserialize (const QByteArray&, QObject*);
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATERACCOUNT_H

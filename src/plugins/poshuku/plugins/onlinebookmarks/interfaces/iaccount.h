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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IACCOUNT_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IACCOUNT_H

#include <QVariant>
#include <QDateTime>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class IAccount
	{
	public:
		virtual ~IAccount () {};

		//
		virtual QObject* GetObject () = 0;

		//
		virtual QObject* GetParentService () const = 0;

		//
		virtual QByteArray GetAccountID () const = 0;


		virtual QString GetLogin () const = 0;

		//
		virtual QString GetPassword () const = 0;

		//
		virtual void SetPassword (const QString&) = 0;

		//
		virtual bool IsSyncing () const = 0;

		//
		virtual bool IsQuickUpload () const = 0;

		//
		virtual QDateTime GetLastUploadDateTime () const = 0;

		//
		virtual QDateTime GetLastDownloadDateTime () const = 0;

		//
		virtual void SetLastUploadDateTime (const QDateTime&) = 0;

		//
		virtual void SetLastDownloadDateTime (const QDateTime&) = 0;

		//
		virtual void SetSyncing (bool) = 0;

		virtual void SetQuickUpload (bool) = 0;
	};
}
}
}

Q_DECLARE_INTERFACE (LeechCraft::Poshuku::OnlineBookmarks::IAccount,
		"org.Deviant.LeechCraft.Poshuku.OnlineBookmarks.IAccount/1.0");

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IACCOUNT_H

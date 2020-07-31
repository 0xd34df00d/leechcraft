/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IACCOUNT_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IACCOUNT_H

#include <QVariant>
#include <QDateTime>
#include <QIcon>

namespace LC
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
		virtual QObject* GetQObject () = 0;

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
		virtual QDateTime GetLastUploadDateTime () const = 0;

		//
		virtual QDateTime GetLastDownloadDateTime () const = 0;

		//
		virtual void SetLastUploadDateTime (const QDateTime&) = 0;

		//
		virtual void SetLastDownloadDateTime (const QDateTime&) = 0;

		//
		virtual void SetSyncing (bool) = 0;
	};
}
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::OnlineBookmarks::IAccount,
		"org.Deviant.LeechCraft.Poshuku.OnlineBookmarks.IAccount/1.0")

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IACCOUNT_H

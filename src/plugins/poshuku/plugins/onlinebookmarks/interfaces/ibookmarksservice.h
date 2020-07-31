/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H

#include <QFlag>
#include <QIcon>
#include <QVariant>
#include <QDateTime>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{

	class IAccount;

	class IBookmarksService
	{
	public:
		virtual ~IBookmarksService () {};

		enum Feature
		{
			FNone = 0,

			FCanRegisterAccount = 0x01
		};
		Q_DECLARE_FLAGS (Features, Feature)

		//
		virtual Features GetFeatures () const = 0;

		virtual QObject* GetQObject () = 0;

		//
		virtual QString GetServiceName () const = 0;

		//
		virtual QIcon GetServiceIcon () const = 0;

		//
		virtual QWidget* GetAuthWidget () = 0;

		//
		virtual void CheckAuthData (const QVariantMap&) = 0;

		//
		virtual void RegisterAccount (const QVariantMap&) = 0;

		//
		virtual void UploadBookmarks (QObject*, const QVariantList&) = 0;

		//
		virtual void DownloadBookmarks (QObject*, const QDateTime& from = QDateTime ()) = 0;

		//
		virtual void saveAccounts () const = 0;

		//
		virtual void removeAccount (QObject*) = 0;

		//
		virtual void accountAdded (QObjectList) = 0;

		//
		virtual void gotBookmarks (QObject*, const QVariantList&) = 0;

		//
		virtual void bookmarksUploaded () = 0;
	};
}
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::Poshuku::OnlineBookmarks::IBookmarksService::Features)
Q_DECLARE_INTERFACE (LC::Poshuku::OnlineBookmarks::IBookmarksService,
					 "org.Deviant.LeechCraft.Poshuku.OnlineBookmarks.IBookmarksService/1.0")

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H

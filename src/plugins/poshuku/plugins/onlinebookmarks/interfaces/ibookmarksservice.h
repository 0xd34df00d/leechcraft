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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H

#include <QFlag>
#include <QIcon>
#include <QVariant>
#include <QDateTime>

namespace LeechCraft
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
		Q_DECLARE_FLAGS (Features, Feature);

		//
		virtual Features GetFeatures () const = 0;

		virtual QObject* GetObject () = 0;

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
	Q_DECLARE_OPERATORS_FOR_FLAGS (IBookmarksService::Features)
}
}
}

Q_DECLARE_INTERFACE (LeechCraft::Poshuku::OnlineBookmarks::IBookmarksService,
					 "org.Deviant.LeechCraft.Poshuku.OnlineBookmarks.IBookmarksService/1.0");

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H

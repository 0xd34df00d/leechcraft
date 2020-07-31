/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>
#include "interfaces/blogique/iaccount.h"

class QTimer;

namespace LC
{
namespace Blogique
{
	struct Entry;
	class BlogiqueWidget;
	class StorageManager;
	class IAccount;
	class IBloggingPlatform;
	class PluginProxy;
	class CommentsManager;

	class Core : public QObject
	{
		Q_OBJECT

		QByteArray UniqueID_;
		ICoreProxy_ptr Proxy_;
		QObjectList BlogPlatformPlugins_;
		std::shared_ptr<PluginProxy> PluginProxy_;
		StorageManager *StorageManager_;
		CommentsManager *CommentsManager_;

		QTimer *AutoSaveTimer_;

		Core ();
		Q_DISABLE_COPY (Core)

	public:
		static Core& Instance ();

		QIcon GetIcon () const;

		void SetCoreProxy (ICoreProxy_ptr proxy);
		ICoreProxy_ptr GetCoreProxy ();

		void Release ();

		void AddPlugin (QObject *plugin);

		QList<IBloggingPlatform*> GetBloggingPlatforms () const;
		QList<IAccount*> GetAccounts () const;

		IAccount* GetAccountByID (const QByteArray& id) const;

		void SendEntity (const Entity& e);
		void DelayedProfilesUpdate ();

		StorageManager* GetStorageManager () const;

		CommentsManager* GetCommentsManager () const;

		BlogiqueWidget* CreateBlogiqueWidget ();
	private:
		void AddBlogPlatformPlugin (QObject *plugin);

	private slots:
		void handleNewBloggingPlatforms (const QObjectList& platforms);
		void addAccount (QObject *accObj);
		void handleAccountRemoved (QObject *accObj);
		void handleAccountValidated (QObject *accObj, bool validated);
		void updateProfiles ();
		void handleEntryPosted (const QList<Entry>& entries);
		void handleEntryRemoved (int itemId);
		void handleEntryUpdated (const QList<Entry>& entries);

		void handleAutoSaveIntervalChanged ();

		void exportBlog ();

	signals:
		void accountAdded (QObject *account);
		void accountRemoved (QObject *account);
		void accountValidated (QObject *account, bool validated);

		void gotEntity (LC::Entity e);

		void addNewTab (const QString& name, QWidget *tab);
		void removeTab (QWidget *tab);
		void changeTabName (QWidget *content, const QString& name);

		void checkAutoSave ();

		void requestEntriesBegin ();

		void entryPosted ();
		void entryRemoved ();

		void tagsUpdated (const QHash<QString, int>& tags);

		void insertTag (const QString& tag);

		void gotError (int errorCode, const QString& errorString,
				const QString& localizedErrorString);
	};
}
}

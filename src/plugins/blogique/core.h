/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QObject>
#include <QSet>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
namespace Blogique
{
	struct Entry;
	class BackupManager;
	class LocalStorage;
	class IAccount;
	class IBloggingPlatform;
	class PluginProxy;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QObjectList BlogPlatformPlugins_;
		std::shared_ptr<PluginProxy> PluginProxy_;
		LocalStorage *Storage_;
		BackupManager *BackupManager_;

		Core ();
		Q_DISABLE_COPY (Core)

	public:
		static Core& Instance ();

		void SetCoreProxy (ICoreProxy_ptr proxy);
		ICoreProxy_ptr GetCoreProxy ();

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject *plugin);

		QList<IBloggingPlatform*> GetBloggingPlatforms () const;
		QList<IAccount*> GetAccounts () const;

		void SendEntity (const Entity& e);
		void DelayedProfilesUpdate ();

		LocalStorage* GetStorage () const;
		BackupManager* GetBackupManager () const;
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

		void handleGotEntries2Backup (const QList<Entry>& events);
		void handleGettingEntries2BackupFinished ();

	signals:
		void accountAdded (QObject *account);
		void accountRemoved (QObject *account);
		void accountValidated (QObject *account, bool validated);

		void gotEntity (LeechCraft::Entity e);
		void delegateEntity (LeechCraft::Entity e, int *id, QObject **obj);

		void addNewTab (const QString& name, QWidget *tab);
		void removeTab (QWidget *tab);

		void gotEntries (const QList<Entry>& entries);
		void storageUpdated ();
	};
}
}

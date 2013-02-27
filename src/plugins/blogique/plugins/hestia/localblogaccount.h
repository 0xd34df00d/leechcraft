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
#include <boost/graph/graph_concepts.hpp>
#include <interfaces/blogique/iaccount.h>

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	class AccountStorage;
	class ImportAccountWidget;
	class LocalBloggingPlatform;

	class LocalBlogAccount : public QObject
							, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IAccount)

		LocalBloggingPlatform *ParentBloggingPlatform_;
		QString Name_;
		bool IsValid_;
		QString DatabasePath_;
		AccountStorage *AccountStorage_;

	public:
		LocalBlogAccount (const QString& name, QObject *parent = 0);

		QObject* GetObject ();
		QObject* GetParentBloggingPlatform () const;
		QString GetAccountName () const;
		QString GetOurLogin () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		void OpenConfigurationDialog ();

		bool IsValid () const;
		QObject* GetProfile ();

		void RemoveEntry (const Entry& entry);
		void UpdateEntry (const Entry& entry);
		QList<QAction*> GetUpdateActions () const;
		void RequestStatistics ();
		void GetEntriesByDate (const QDate& date);
		void GetLastEntries (int count);

		void FillSettings (ImportAccountWidget *widget);
		void Init ();

		QByteArray Serialize () const;
		static LocalBlogAccount* Deserialize (const QByteArray& data, QObject *parent);
	private:
		void Validate ();

	public slots:
		void updateProfile ();
		void submit (const Entry& event);
		void backup ();

	signals:
		void accountRenamed (const QString& newName);
		void accountSettingsChanged ();
		void accountValidated (bool validated);

		void entryPosted (const QList<Entry>& entries);
		void entryRemoved (int itemId);
		void entryUpdated (const QList<Entry>& entries);
		void gotEntries2Backup (const QList<Entry>& entries);
		void gettingEntries2BackupFinished ();
		void gotBlogStatistics (const QMap<QDate, int>& statistics);

	};
}
}
}


/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SNAILS_ACCOUNTFOLDERMANAGER_H
#define PLUGINS_SNAILS_ACCOUNTFOLDERMANAGER_H
#include <QObject>
#include <QStringList>
#include <QHash>

namespace LeechCraft
{
namespace Snails
{
	class Account;

	class AccountFolderManager : public QObject
	{
		Q_OBJECT

		friend class Account;
	public:
		enum FolderFlag
		{
			FolderSyncable = 0x01,
			FolderOutgoing = 0x02
		};

		Q_DECLARE_FLAGS (FolderFlags, FolderFlag);
	private:
		QList<QStringList> Folders_;

		QHash<QStringList, FolderFlags> Folder2Flags_;
	public:
		AccountFolderManager (QObject* = 0);

		QList<QStringList> GetFolders () const;
		QList<QStringList> GetSyncFolders () const;
		FolderFlags GetFolderFlags (const QStringList&) const;
	private:
		void ClearFolderFlags ();
		void AppendFolderFlags (const QStringList&, FolderFlag);

		void SetFolders (const QList<QStringList>&);

		QByteArray Serialize () const;
		void Deserialize (const QByteArray&);
	signals:
		void foldersUpdated ();
	};
}
}

#endif

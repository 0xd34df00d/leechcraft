/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QHash>
#include "folder.h"

namespace LC
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
		QList<Folder> Folders_;

		QHash<QStringList, FolderFlags> Folder2Flags_;
	public:
		using QObject::QObject;

		QList<Folder> GetFolders () const;
		QList<QStringList> GetFoldersPaths () const;
		QList<QStringList> GetSyncFolders () const;
		FolderFlags GetFolderFlags (const QStringList&) const;
	private:
		void ClearFolderFlags ();
		void AppendFolderFlags (const QStringList&, FolderFlag);

		void AddFolder (const QStringList&);
		void SetFolders (const QList<Folder>&);

		QByteArray Serialize () const;
		void Deserialize (const QByteArray&);
	signals:
		void foldersUpdated ();
	};
}
}

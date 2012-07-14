/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "accountfoldermanager.h"
#include <stdexcept>
#include "common.h"

namespace LeechCraft
{
namespace Snails
{
	AccountFolderManager::AccountFolderManager (QObject *parent)
	: QObject (parent)
	{
	}

	QList<QStringList> AccountFolderManager::GetFolders () const
	{
		return Folders_;
	}

	QList<QStringList> AccountFolderManager::GetSyncFolders () const
	{
		QList<QStringList> result;

		std::copy_if (Folders_.begin (), Folders_.end (), std::back_inserter (result),
				[this] (const QStringList& folder) { return Folder2Flags_ [folder] & FolderSyncable; });

		return result;
	}

	AccountFolderManager::FolderFlags AccountFolderManager::GetFolderFlags (const QStringList& folder) const
	{
		return Folder2Flags_ [folder];
	}

	void AccountFolderManager::ClearFolderFlags ()
	{
		Folder2Flags_.clear ();
	}

	void AccountFolderManager::AppendFolderFlags (const QStringList& folder, FolderFlag flags)
	{
		Folder2Flags_ [folder] |= flags;
	}

	void AccountFolderManager::SetFolders (const QList<QStringList>& folders)
	{
		if (folders == Folders_)
			return;

		Folders_ = folders;

		emit foldersUpdated ();
	}

	QByteArray AccountFolderManager::Serialize () const
	{
		QByteArray result;

		QHash<QStringList, int> flags;
		Q_FOREACH (const auto key, Folder2Flags_.keys ())
			flags [key] = Folder2Flags_ [key];

		QDataStream out (&result, QIODevice::WriteOnly);
		out << static_cast<quint8> (1);
		out << Folders_
			<< flags;

		return result;
	}

	void AccountFolderManager::Deserialize (const QByteArray& arr)
	{
		QDataStream in (arr);
		quint8 version = 0;
		in >> version;

		if (version < 1 || version > 1)
			throw std::runtime_error (qPrintable ("Unknown folder manager version " + QString::number (version)));

		QHash<QStringList, int> flags;
		in >> Folders_
			>> flags;

		Q_FOREACH (const auto key, flags.keys ())
			Folder2Flags_ [key] = static_cast<FolderFlags> (flags [key]);
	}
}
}

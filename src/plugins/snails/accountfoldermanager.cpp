/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountfoldermanager.h"
#include <stdexcept>
#include <QDataStream>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "common.h"

namespace LC
{
namespace Snails
{
	QList<Folder> AccountFolderManager::GetFolders () const
	{
		return Folders_;
	}

	QList<QStringList> AccountFolderManager::GetFoldersPaths() const
	{
		QList<QStringList> result;
		result.reserve (Folders_.size ());
		for (const auto& folder : Folders_)
			result << folder.Path_;
		return result;
	}

	QList<QStringList> AccountFolderManager::GetSyncFolders () const
	{
		QList<QStringList> result;
		for (const auto& folder : Folders_)
			if (Folder2Flags_ [folder.Path_] & FolderSyncable)
				result << folder.Path_;
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

	void AccountFolderManager::AddFolder (const QStringList& newPath)
	{
		if (std::any_of (Folders_.begin (), Folders_.end (),
				[&newPath] (const auto& folder) { return folder.Path_ == newPath; }))
			return;

		Folders_ << Folder { newPath, FolderType::Other };
		emit foldersUpdated ();
	}

	void AccountFolderManager::SetFolders (const QList<Folder>& folders)
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
		for (const auto& pair : Util::Stlize (Folder2Flags_))
			flags [pair.first] = pair.second;

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

		Folders_.erase (std::remove_if (Folders_.begin (), Folders_.end (),
					[] (const Folder& folder)
					{
						return folder.Path_.isEmpty () ||
								std::all_of (folder.Path_.begin (), folder.Path_.end (),
										[] (const QString& elem) { return elem.isEmpty (); });
					}),
				Folders_.end ());

		for (const auto& pair : Util::Stlize (flags))
			Folder2Flags_ [pair.first] = static_cast<FolderFlags> (pair.second);
	}
}
}

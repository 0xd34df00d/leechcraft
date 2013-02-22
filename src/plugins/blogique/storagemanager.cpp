/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include "storagemanager.h"

namespace LeechCraft
{
namespace Blogique
{
	StorageManager::StorageManager (QObject *parent)
	: QObject (parent)
	{
	}

	void StorageManager::AddAccount (const QByteArray& accounId)
	{
// 		Util::DBLock lock (DB_);
// 		lock.Init ();
//
// 		AddAccount_.bindValue (":account_id", QString::fromUtf8 (accounId));
// 		if (!AddAccount_.exec ())
// 		{
// 			Util::DBLock::DumpError (AddAccount_);
// 			throw std::runtime_error ("unable to add account");
// 		}
//
// 		lock.Good ();
	}

	void StorageManager::saveNewDraft (const Entry& e)
	{

	}

}
}
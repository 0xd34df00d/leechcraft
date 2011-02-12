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

#include "storagethread.h"
#include "storage.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	StorageThread::StorageThread (QObject *parent)
	: QThread (parent)
	{
	}

	Storage* StorageThread::GetStorage ()
	{
		return Storage_.get ();
	}
	
	void StorageThread::run ()
	{
		Storage_.reset (new Storage);

		connect (Storage_.get (),
				SIGNAL (gotOurAccounts (const QStringList&)),
				Core::Instance ().get (),
				SIGNAL (gotOurAccounts (const QStringList&)),
				Qt::QueuedConnection);
		connect (Storage_.get (),
				SIGNAL (gotUsersForAccount (const QStringList&, const QString&)),
				Core::Instance ().get (),
				SIGNAL (gotUsersForAccount (const QStringList&, const QString&)),
				Qt::QueuedConnection);
		connect (Storage_.get (),
				SIGNAL (gotChatLogs (const QString&, const QString&, int, int, const QVariant&)),
				Core::Instance ().get (),
				SIGNAL (gotChatLogs (const QString&, const QString&, int, int, const QVariant&)),
				Qt::QueuedConnection);

		QThread::run ();
		Storage_.reset ();
	}
}
}
}
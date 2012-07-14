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

#include "todomanager.h"
#include <QStandardItemModel>
#include "todostorage.h"
#include "storagemodel.h"
#include "notificationsmanager.h"
#include "core.h"

namespace LeechCraft
{
namespace Otlozhu
{
	TodoManager::TodoManager (const QString& ctx, QObject *parent)
	: QObject (parent)
	, Context_ (ctx)
	, Storage_ (new TodoStorage (ctx, this))
	, Model_ (new StorageModel (this))
	, NotifMgr_ (new NotificationsManager (Storage_))
	{
		Model_->SetStorage (Storage_);

		connect (NotifMgr_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	TodoStorage* TodoManager::GetTodoStorage () const
	{
		return Storage_;
	}

	QAbstractItemModel* TodoManager::GetTodoModel () const
	{
		return Model_;
	}
}
}

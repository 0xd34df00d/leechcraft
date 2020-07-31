/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "todomanager.h"
#include <QStandardItemModel>
#include "todostorage.h"
#include "storagemodel.h"
#include "notificationsmanager.h"
#include "core.h"

namespace LC
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
				SIGNAL (gotEntity (LC::Entity)),
				this,
				SIGNAL (gotEntity (LC::Entity)));
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

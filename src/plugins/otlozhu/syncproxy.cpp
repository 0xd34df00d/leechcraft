/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncproxy.h"
#include <QtDebug>
#include <laretz/operation.h>
#include <laretz/opsummer.h>
#include "core.h"
#include "todomanager.h"
#include "todostorage.h"
#include "stager.h"

namespace LC
{
namespace Otlozhu
{
	SyncProxy::SyncProxy (QObject *parent)
	: QObject (parent)
	{
	}

	QObject* SyncProxy::GetQObject ()
	{
		return this;
	}

	namespace
	{
		std::string ToStdStr (const QString& str)
		{
			return str.toUtf8 ().constData ();
		}
	}

	QList<Laretz::Operation> SyncProxy::GetAllOps () const
	{
		Core::Instance ().GetStager ()->Enable ();

		std::vector<Laretz::Item> items;

		const auto todoMgr = Core::Instance ().GetTodoManager ();
		for (const auto& todoItem : todoMgr->GetTodoStorage ()->GetAllItems ())
		{
			Laretz::Item it (ToStdStr (todoItem->GetID ()), 0);
			Util::Sync::FillItem (it, todoItem->ToMap ());
			items.push_back (std::move (it));
		}

		return { { Laretz::OpType::Append, items } };
	}

	QList<Laretz::Operation> SyncProxy::GetNewOps () const
	{
		auto stager = Core::Instance ().GetStager ();
		return stager->IsEnabled () ? stager->GetStagedOps () : GetAllOps ();
	}

	void SyncProxy::Merge (QList<Laretz::Operation>&, const QList<Laretz::Operation>& theirs)
	{
		auto guard = Core::Instance ().GetStager ()->EnterMergeMode ();

		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		for (const auto& op : theirs)
		{
			const auto& items = op.getItems ();
			switch (op.getType ())
			{
			case Laretz::OpType::Fetch:
				for (const auto& item : items)
				{
					const auto pos = storage->FindItem (QString::fromUtf8 (item.getId ().c_str ()));
					if (pos == -1)
					{
						TodoItem_ptr todo (new TodoItem (QString::fromUtf8 (item.getId ().c_str ())));
						todo->ApplyDiff (Util::Sync::ItemToMap (item));
						storage->AddItem (todo);
					}
					else
					{
						auto todo = storage->GetItemAt (pos);
						todo->ApplyDiff (Util::Sync::ItemToMap (item));
						storage->HandleUpdated (todo);
					}
				}
				break;
			case Laretz::OpType::Delete:
				for (const auto& item : items)
					storage->RemoveItem (QString::fromUtf8 (item.getId ().c_str ()));
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown operation type"
						<< static_cast<int> (op.getType ());
				break;
			}
		}
	}
}
}

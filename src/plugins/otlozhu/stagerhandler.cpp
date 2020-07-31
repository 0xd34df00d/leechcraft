/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stagerhandler.h"
#include <laretz/item.h>
#include <laretz/operation.h>
#include "stager.h"
#include "core.h"
#include "todomanager.h"
#include "todostorage.h"

namespace LC
{
namespace Otlozhu
{
	StagerHandler::StagerHandler (QObject *parent)
	: QObject (parent)
	{
	}

	void StagerHandler::handleItemAdded (int pos)
	{
		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();

		auto todoItem = storage->GetItemAt (pos);
		const auto& id = todoItem->GetID ();

		Laretz::Item item (id.toStdString (), 0);
		Util::Sync::FillItem (item, todoItem->ToMap ());

		Core::Instance ().GetStager ()->Add ({ { Laretz::OpType::Append, { item } } });
	}

	void StagerHandler::handleItemRemoved (int pos)
	{
		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		const auto& id = storage->GetItemAt (pos)->GetID ();

		Core::Instance ().GetStager ()->Add ({ { Laretz::OpType::Delete, { { id.toStdString (), 0 } } } });
	}

	void StagerHandler::handleItemDiffGenerated (const QString& id, const QVariantMap& news)
	{
		Laretz::Item item (id.toStdString (), 0);
		Util::Sync::FillItem (item, news);

		Core::Instance ().GetStager ()->Add ({ { Laretz::OpType::Modify, { item } } });
	}
}
}

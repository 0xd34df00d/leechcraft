/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemsmodeldecorator.h"
#include <QMetaObject>
#include <QModelIndex>
#include <interfaces/aggregator/iitemsmodel.h>

namespace LC::Aggregator
{
	ItemsModelDecorator::ItemsModelDecorator (IItemsModel& model)
	: Model_ { model }
	{
	}

	void ItemsModelDecorator::Reset (IDType_t channelId)
	{
		QMetaObject::invokeMethod (&dynamic_cast<QObject&> (Model_),
				[=] { Model_.Reset (channelId); },
				Qt::QueuedConnection);
	}

	void ItemsModelDecorator::Selected (const QModelIndex& index)
	{
		QMetaObject::invokeMethod (&dynamic_cast<QObject&> (Model_),
				[=] { Model_.Selected (index); },
				Qt::QueuedConnection);
	}
}

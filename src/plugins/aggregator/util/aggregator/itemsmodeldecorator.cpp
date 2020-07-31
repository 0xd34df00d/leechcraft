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

namespace LC
{
namespace Aggregator
{
	ItemsModelDecorator::ItemsModelDecorator (QObject *model)
	: Model_ { model }
	{
	}

	void ItemsModelDecorator::Reset (IDType_t channelId, IDType_t feedId)
	{
		QMetaObject::invokeMethod (Model_,
				"reset",
				Qt::QueuedConnection,
				Q_ARG (IDType_t, channelId),
				Q_ARG (IDType_t, feedId));
	}

	void ItemsModelDecorator::Selected (const QModelIndex& index)
	{
		QMetaObject::invokeMethod (Model_,
				"selected",
				Qt::QueuedConnection,
				Q_ARG (QModelIndex, index));
	}
}
}

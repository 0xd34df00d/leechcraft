/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <Qt>
#include "common.h"

class QModelIndex;

namespace LC
{
namespace Aggregator
{
	class IItemsModel
	{
	public:
		virtual ~IItemsModel () {}

		enum ItemRole
		{
			IsRead = Qt::UserRole + 1,
			ItemId,
			ItemShortDescr,
			ItemCategories,
		};

		virtual void reset (IDType_t channelId) = 0;
		virtual void selected (const QModelIndex&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Aggregator::IItemsModel,
		"org.Deviant.LeechCraft.Aggregator.IItemsModel/1.0")

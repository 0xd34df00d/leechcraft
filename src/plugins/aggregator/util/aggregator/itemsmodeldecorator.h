/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <common.h>

class QModelIndex;

namespace LC::Aggregator
{
	class IItemsModel;

	class ItemsModelDecorator
	{
		IItemsModel& Model_;
	public:
		explicit ItemsModelDecorator (IItemsModel&);

		void Reset (IDType_t);
		void Selected (const QModelIndex&);
	};
}

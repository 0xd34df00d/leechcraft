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

namespace LC
{
namespace Aggregator
{
	class ItemsModelDecorator
	{
		QObject * const Model_;
	public:
		ItemsModelDecorator (QObject*);

		void Reset (IDType_t, IDType_t);
		void Selected (const QModelIndex&);
	};
}
}

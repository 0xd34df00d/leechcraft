/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class SDSession;

	class SDModel final : public QStandardItemModel
	{
		SDSession * const Session_;
	public:
		SDModel (SDSession*);
		
		bool canFetchMore (const QModelIndex&) const override;
		void fetchMore (const QModelIndex&) override;
		bool hasChildren (const QModelIndex&) const override;
	};
}
}
}

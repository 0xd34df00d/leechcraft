/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC::Azoth::Acetamide
{
	class ChannelsListFilterProxyModel : public QSortFilterProxyModel
	{
	public:
		explicit ChannelsListFilterProxyModel (QObject *parent = nullptr);
	protected:
		bool filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const override;
	};
}

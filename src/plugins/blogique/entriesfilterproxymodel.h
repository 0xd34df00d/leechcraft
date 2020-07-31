/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC
{
namespace Blogique
{
	class EntriesFilterProxyModel : public QSortFilterProxyModel
	{
	public:
		EntriesFilterProxyModel (QObject *parent = 0);
	protected:
		bool filterAcceptsRow (int sourceRow,
				const QModelIndex& sourceParent) const;
	};
}
}


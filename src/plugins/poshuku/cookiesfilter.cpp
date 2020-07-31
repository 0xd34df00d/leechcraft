/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cookiesfilter.h"

namespace LC
{
namespace Poshuku
{
	CookiesFilter::CookiesFilter (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
	}
	
	bool CookiesFilter::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		return parent.isValid () ? true : QSortFilterProxyModel::filterAcceptsRow (row, parent);
	}
}
}

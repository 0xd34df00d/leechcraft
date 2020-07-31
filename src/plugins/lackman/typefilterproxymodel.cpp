/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "typefilterproxymodel.h"
#include <QtDebug>
#include "packagesmodel.h"

namespace LC
{
namespace LackMan
{
	TypeFilterProxyModel::TypeFilterProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	, Mode_ (FilterMode::All)
	{
	}

	void TypeFilterProxyModel::SetFilterMode (FilterMode fm)
	{
		Mode_ = fm;
		invalidateFilter ();
	}

	bool TypeFilterProxyModel::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		const auto& index = sourceModel ()->index (row, 0, parent);
		switch (Mode_)
		{
		case FilterMode::Installed:
			return index.data (PackagesModel::PMRInstalled).toBool ();
		case FilterMode::NotInstalled:
			return !index.data (PackagesModel::PMRInstalled).toBool ();
		case FilterMode::Upgradable:
			return index.data (PackagesModel::PMRUpgradable).toBool ();
		case FilterMode::All:
			return true;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown filter mode"
				<< static_cast<int> (Mode_);
		return true;
	}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC
{
namespace LMP
{
	class CollectionSorterModel : public QSortFilterProxyModel
	{
		Q_OBJECT

		bool UseThe_;
	public:
		CollectionSorterModel (QObject*);
	protected:
		bool lessThan (const QModelIndex&, const QModelIndex&) const;
	private slots:
		void handleUseTheChanged ();
	};
}
}

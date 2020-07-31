/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SORTFILTERPROXYMODEL_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class SortFilterProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		SortFilterProxyModel (QObject *parent = 0);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const;
	};
}
}
}
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SORTFILTERPROXYMODEL_H

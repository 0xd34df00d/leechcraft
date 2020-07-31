/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsproxymodel.h"
#include "blogiquewidget.h"

namespace LC
{
namespace Blogique
{
	TagsProxyModel::TagsProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
		setFilterCaseSensitivity (Qt::CaseInsensitive);
	}

	bool TagsProxyModel::filterAcceptsRow (int sourceRow,
			const QModelIndex& sourceParent) const
	{
		QModelIndex index = sourceModel ()->index (sourceRow, 0, sourceParent);

		return sourceModel ()->data (index).toString ()
				.startsWith (filterRegExp ().pattern ());
	}

	bool TagsProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		const int leftData = sourceModel ()->data (left, BlogiqueWidget::TagFrequency)
				.toInt ();
		const int rightData = sourceModel ()->data (right, BlogiqueWidget::TagFrequency)
				.toInt ();

		return leftData > rightData;
	}

	int TagsProxyModel::GetCount () const
	{
		return rowCount ();
	}

	QString TagsProxyModel::GetTagName (int idx)
	{
		return mapToSource (index (idx, 0)).data ().toString ();
	}

	void TagsProxyModel::countUpdated ()
	{
		emit countChanged ();
	}
}
}

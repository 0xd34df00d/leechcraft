/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "summarytagsfilter.h"
#include <QStringList>
#include <interfaces/structures.h>

namespace LC
{
namespace Summary
{
	SummaryTagsFilter::SummaryTagsFilter (QObject *parent)
	: Util::TagsFilterModel (parent)
	{
		setProperty ("__LeechCraft_own_core_model", true);
	}

	QVariant SummaryTagsFilter::data (const QModelIndex& index, int role) const
	{
		const auto& result = QSortFilterProxyModel::data (index, role);
		if (role != Qt::ToolTipRole)
			return result;

		if (result.isValid ())
			return result;

		return QSortFilterProxyModel::data (index, Qt::DisplayRole);
	}

	QStringList SummaryTagsFilter::GetTagsForIndex (int index) const
	{
		const auto model = sourceModel ();
		if (!model)
			return {};

		return model->data (model->index (index, 0), RoleTags).toStringList ();
	}
}
}

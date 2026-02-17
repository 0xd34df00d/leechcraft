/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "jobholderrepresentationmodel.h"
#include <QMenu>
#include <QToolBar>
#include <interfaces/ijobholder.h>
#include "common.h"

namespace LC::Aggregator
{
	JobHolderRepresentationModel::JobHolderRepresentationModel (const Deps& deps, QObject *parent)
	: QSortFilterProxyModel { parent }
	, Deps_ { deps }
	{
	}

	QVariant JobHolderRepresentationModel::data (const QModelIndex& index, int role) const
	{
		switch (static_cast<JobHolderRole> (role))
		{
		case JobHolderRole::RowInfo:
			return QVariant::fromValue<RowInfo> ({
					.Name_ = index.data (ChannelRoles::ChannelTitle).toString (),
					.Specific_ = NewsInfo
					{
						.Count_ = index.data (ChannelRoles::UnreadCount).toInt (),
						.LastUpdate_ = index.data (ChannelRoles::LastUpdate).toDateTime (),
					}
				});
		}

		return QSortFilterProxyModel::data (index, role);
	}

	bool JobHolderRepresentationModel::filterAcceptsRow (int row, const QModelIndex&) const
	{
		// The row won't show up anyway in the job list if it was empty, so
		// we can just check if it has unread items or selected. Later means
		// that user's just clicked last unread item there.
		const auto srcIdx = sourceModel ()->index (row, 0);
		return srcIdx.data (ChannelRoles::UnreadCount).toInt () ||
				srcIdx.data (ChannelRoles::ErrorCount).toInt () ||
				srcIdx.data (Deps_.SelectedRole_).toBool ();
	}
}

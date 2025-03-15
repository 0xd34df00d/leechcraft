/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "jobholderrepresentation.h"
#include <QMenu>
#include <QToolBar>
#include "common.h"

namespace LC::Aggregator
{
	JobHolderRepresentation::JobHolderRepresentation (const Deps& deps, QObject *parent)
	: QSortFilterProxyModel { parent }
	, Deps_ { deps }
	{
		setDynamicSortFilter (true);
	}

	QVariant JobHolderRepresentation::data (const QModelIndex& index, int role) const
	{
		switch (role)
		{
		case RoleControls:
			return QVariant::fromValue<QToolBar*> (&Deps_.Toolbar_);
		case RoleAdditionalInfo:
			return QVariant::fromValue<QWidget*> (&Deps_.DetailsWidget_);
		case RoleContextMenu:
			return QVariant::fromValue<QMenu*> (&Deps_.RowMenu_);
		default:
			return QSortFilterProxyModel::data (index, role);
		}
	}
	
	bool JobHolderRepresentation::filterAcceptsRow (int row, const QModelIndex&) const
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

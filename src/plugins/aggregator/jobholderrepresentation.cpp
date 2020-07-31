/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "jobholderrepresentation.h"
#include <QTimer>
#include <QtDebug>
#include "aggregator.h"

namespace LC
{
namespace Aggregator
{
	JobHolderRepresentation::JobHolderRepresentation (QObject *parent)
	: QSortFilterProxyModel (parent)
	{
		setDynamicSortFilter (true);
	}
	
	QModelIndex JobHolderRepresentation::SelectionChanged (const QModelIndex& index)
	{
		if (index.isValid ())
			Selected_ = mapToSource (index);
		else
			Selected_ = QModelIndex ();
		invalidateFilter ();
		return mapFromSource (Selected_);
	}
	
	bool JobHolderRepresentation::filterAcceptsRow (int row, const QModelIndex&) const
	{
		// The row won't show up anyway in the job list if it was empty, so
		// we can just check if it has unread items or selected. Later means
		// that user's just clicked last unread item there.
		auto srcIdx = sourceModel ()->index (row, 0);
		return srcIdx.data (ChannelRoles::UnreadCount).toInt () ||
				srcIdx.data (ChannelRoles::ErrorCount).toInt () ||
				(Selected_.isValid () && row == Selected_.row ());
	}
}
}

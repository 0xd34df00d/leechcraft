/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelsexportmodel.h"

namespace LC::Aggregator
{
	QVariant ChannelsExportModel::data (const QModelIndex& index, int role) const
	{
		switch (role)
		{
		case Qt::BackgroundRole:
		case Qt::ForegroundRole:
			return {};
		}

		return CheckableProxyModel::data (index, role);
	}
}

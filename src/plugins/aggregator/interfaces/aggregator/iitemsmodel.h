/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <Qt>
#include <QVector>
#include "common.h"

class QAbstractItemModel;
class QModelIndex;

namespace LC
{
namespace Aggregator
{
	class IItemsModel
	{
	public:
		virtual ~IItemsModel () = default;

		enum ItemRole
		{
			IsRead = Qt::UserRole + 1,
			ItemId,
			ItemShortDescr,
			ItemCategories,
			ItemImportant,
			ItemChannelId,

			FullItem,
		};

		constexpr static auto MaxItemRole = FullItem;

		virtual QAbstractItemModel& GetQModel () = 0;

		virtual void SetChannels (const QVector<IDType_t>& channelIds) = 0;

		virtual QList<QModelIndex> FindItems (const QSet<IDType_t>&) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Aggregator::IItemsModel,
		"org.Deviant.LeechCraft.Aggregator.IItemsModel/1.0")

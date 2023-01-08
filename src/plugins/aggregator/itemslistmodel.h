/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include <QSet>
#include <QPair>
#include <QIcon>
#include <QThreadStorage>
#include "interfaces/aggregator/iitemsmodel.h"
#include "item.h"
#include "channel.h"
#include "storagebackend.h"

class IIconThemeManager;

namespace LC
{
namespace Aggregator
{
	class ItemsListModel : public QAbstractItemModel
						 , public IItemsModel
	{
		Q_OBJECT
		Q_INTERFACES (LC::Aggregator::IItemsModel)

		QStringList ItemHeaders_;
		items_shorts_t CurrentItems_;
		IDType_t CurrentChannel_ = -1;

		const QIcon StarredIcon_;
		const QIcon UnreadIcon_;
		const QIcon ReadIcon_;

		mutable QThreadStorage<StorageBackend_ptr> SB_;
	public:
		explicit ItemsListModel (IIconThemeManager*, QObject* = nullptr);

		QAbstractItemModel& GetQModel () override;

		const IDType_t& GetCurrentChannel () const;
		void Selected (const QModelIndex&) override;
		const ItemShort& GetItem (const QModelIndex&) const;
		const items_shorts_t& GetAllItems () const;
		void Reset (IDType_t) override;
		void Reset (const QList<IDType_t>&);
		void RemoveItems (const QSet<IDType_t>&);
		void ItemDataUpdated (const Item&);

		int columnCount (const QModelIndex& = QModelIndex ()) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = QModelIndex ()) const override;
	private:
		StorageBackend_ptr GetSB () const;
		void HandleItemReadStatusUpdated (IDType_t, IDType_t, bool);
	};
}
}

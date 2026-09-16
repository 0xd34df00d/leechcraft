/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemselectiontracker.h"
#include <QAbstractItemView>
#include <QTimer>
#include <util/sll/prelude.h>
#include "interfaces/aggregator/iitemsmodel.h"
#include "components/storage/storagebackendmanager.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	namespace
	{
		void RunMarkAsRead (const QSet<ItemSelectionTracker::SelectedItem>& items)
		{
			const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			const auto& idsList = Util::MapAs<QList> (items,
					[] (const ItemSelectionTracker::SelectedItem& item) { return SQLStorageBackend::UnreadItemId { item.Channel_, item.Item_ }; });
			sb->SetItemsUnread (idsList, false);
		}
	}

	ItemSelectionTracker::SelectedItem ItemSelectionTracker::SelectedItem::FromIndex (const QModelIndex& idx)
	{
		return
		{
			.Channel_ = idx.data (IItemsModel::ItemRole::ItemChannelId).value<IDType_t> (),
			.Item_ = idx.data (IItemsModel::ItemRole::ItemId).value<IDType_t> (),
		};
	}

	std::size_t qHash (const ItemSelectionTracker::SelectedItem& item, size_t seed)
	{
		return qHashMulti (seed, item.Channel_, item.Item_);
	}

	ItemSelectionTracker::ItemSelectionTracker (QAbstractItemView& view, ItemActions& actions, QObject *parent)
	: QObject { parent }
	, Tracker_ { view }
	, Actions_ { actions }
	{
		ReadMarkTimer_.callOnTimeout (this, [this] { RunMarkAsRead (CurrentItems_); });
		ReadMarkTimer_.setSingleShot (true);

		connect (&Tracker_,
				&Util::ViewSelectionTracker::gestureStarted,
				&ReadMarkTimer_,
				&QTimer::stop);
		connect (&Tracker_,
				&Util::ViewSelectionTracker::selectionChanging,
				this,
				[this]
				{
					ReadMarkTimer_.stop ();
					emit refreshItemDisplay ();
				});
		connect (&Tracker_,
				&Util::ViewSelectionTracker::selectionSettled,
				this,
				&ItemSelectionTracker::HandleSelectionChanged);

		connect (view.model (),
				&QAbstractItemModel::dataChanged,
				this,
				[&] (const QModelIndex& from, const QModelIndex& to)
				{
					for (int row = from.row (); row <= to.row (); ++row)
						if (CurrentItems_.contains (SelectedItem::FromIndex (from.siblingAtRow (row))))
						{
							Actions_.HandleSelectionChanged (view.selectionModel ()->selectedRows ());
							return;
						}
				});
	}

	QSet<IDType_t> ItemSelectionTracker::GetSelectedItems () const
	{
		return Util::Map (CurrentItems_, &SelectedItem::Item_);
	}

	void ItemSelectionTracker::HandleSelectionChanged (const QList<QModelIndex>& rows)
	{
		Actions_.HandleSelectionChanged (rows);

		if (const auto isUnread = [] (const QModelIndex& row) { return !row.data (IItemsModel::ItemRole::IsRead).toBool (); };
			std::ranges::any_of (rows, isUnread))
			RearmMarkTimer ();

		if (auto currentItems = Util::MapAs<QSet> (rows, &SelectedItem::FromIndex);
			currentItems != CurrentItems_)
		{
			CurrentItems_ = std::move (currentItems);

			emit selectionChanged (GetSelectedItems ());
		}
	}

	void ItemSelectionTracker::RearmMarkTimer ()
	{
		const auto timeout = XmlSettingsManager::Instance ().property ("MarkAsReadTimeout").toInt ();
		ReadMarkTimer_.start (std::chrono::seconds { timeout });
	}
}

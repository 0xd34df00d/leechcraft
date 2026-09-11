/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemselectiontracker.h"
#include <QAbstractItemView>
#include <QMouseEvent>
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
			for (const auto& item : items)
				sb->SetItemUnread (item.Channel_, item.Item_, false);
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
	, View_ { view }
	, Actions_ { actions }
	, ReadMarkTimer_ { *new QTimer { this } }
	{
		ReadMarkTimer_.callOnTimeout (this, [this] { RunMarkAsRead (CurrentItems_); });
		ReadMarkTimer_.setSingleShot (true);

		const auto sm = view.selectionModel ();

		View_.viewport ()->installEventFilter (this);

		connect (sm,
				&QItemSelectionModel::selectionChanged,
				this,
				&ItemSelectionTracker::HandleImmediateSelectionChange);
		connect (view.model (),
				&QAbstractItemModel::modelReset,
				this,
				&ItemSelectionTracker::HandleImmediateSelectionChange);

		connect (view.model (),
				&QAbstractItemModel::dataChanged,
				this,
				[&, sm] (const QModelIndex& from, const QModelIndex& to)
				{
					for (int row = from.row (); row <= to.row (); ++row)
						if (CurrentItems_.contains (SelectedItem::FromIndex (from.siblingAtRow (row))))
						{
							Actions_.HandleSelectionChanged (sm->selectedRows ());
							return;
						}
				});
	}

	QSet<IDType_t> ItemSelectionTracker::GetSelectedItems () const
	{
		return Util::Map (CurrentItems_, &SelectedItem::Item_);
	}

	void ItemSelectionTracker::SetTapeMode (bool tape)
	{
		TapeMode_ = tape;
	}

	bool ItemSelectionTracker::eventFilter (QObject*, QEvent *ev)
	{
		switch (ev->type ())
		{
		case QEvent::MouseButtonDblClick:
		case QEvent::MouseButtonPress:
			if (static_cast<QMouseEvent*> (ev)->button () == Qt::LeftButton)
			{
				GestureActive_ = true;
				ReadMarkTimer_.stop ();
			}
			break;
		case QEvent::MouseButtonRelease:
			if (static_cast<QMouseEvent*> (ev)->button () == Qt::LeftButton)
				EndGesture ();
			break;
		case QEvent::UngrabMouse:
			EndGesture ();
			break;
		default:
			break;
		}

		return false;
	}

	void ItemSelectionTracker::EndGesture ()
	{
		if (std::exchange (GestureActive_, false))
			ScheduleSyncToSelection ();
	}

	void ItemSelectionTracker::HandleImmediateSelectionChange ()
	{
		if (!ScheduledSyncToSelection_)
			ScheduleSyncToSelection ();
	}

	void ItemSelectionTracker::ScheduleSyncToSelection ()
	{
		ScheduledSyncToSelection_ = true;
		QTimer::singleShot (0, this, &ItemSelectionTracker::SyncToSelection);
	}

	void ItemSelectionTracker::SyncToSelection ()
	{
		if (GestureActive_ || !std::exchange (ScheduledSyncToSelection_, false))
			return;

		const auto sm = View_.selectionModel ();
		const auto& rows = sm->selectedRows ();
		Actions_.HandleSelectionChanged (rows);
		if (!TapeMode_)
			emit refreshItemDisplay ();

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
		ReadMarkTimer_.stop ();

		if (TapeMode_)
			return;

		const auto timeout = XmlSettingsManager::Instance ().property ("MarkAsReadTimeout").toInt ();
		ReadMarkTimer_.start (std::chrono::seconds { timeout });
	}
}

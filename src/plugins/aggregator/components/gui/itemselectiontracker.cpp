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
#include "interfaces/aggregator/iitemsmodel.h"
#include "components/storage/storagebackendmanager.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	namespace
	{
		void RunMarkAsRead (const QModelIndex& index)
		{
			if (!index.isValid () || index.data (IItemsModel::ItemRole::IsRead).toBool ())
				return;

			const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			const auto channelId = index.data (IItemsModel::ItemRole::ItemChannelId).value<IDType_t> ();
			const auto itemId = index.data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
			sb->SetItemUnread (channelId, itemId, false);
		}
	}

	ItemSelectionTracker::ItemSelectionTracker (QAbstractItemView& view, ItemActions& actions, QObject *parent)
	: QObject { parent }
	, View_ { view }
	, Actions_ { actions }
	, ReadMarkTimer_ { *new QTimer { this } }
	{
		ReadMarkTimer_.callOnTimeout (this, [this] { RunMarkAsRead (View_.currentIndex ()); });
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
					{
						const auto changedItemId = from.siblingAtRow (row).data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
						if (CurrentItems_.contains (changedItemId))
						{
							Actions_.HandleSelectionChanged (sm->selectedRows ());
							return;
						}
					}
				});
	}

	QSet<IDType_t> ItemSelectionTracker::GetSelectedItems () const
	{
		return CurrentItems_;
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
		if (std::exchange (GestureActive_, false) && ScheduledSyncToSelection_)
			QTimer::singleShot (0, this, &ItemSelectionTracker::SyncToSelection);
	}

	void ItemSelectionTracker::HandleImmediateSelectionChange ()
	{
		if (std::exchange (ScheduledSyncToSelection_, true))
			return;

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

		SaveCurrentItems (rows);
		emit selectionChanged (CurrentItems_);

		if (const auto& curIdx = sm->currentIndex ();
			curIdx.isValid ())
			MarkRowAsRead (curIdx);
	}

	void ItemSelectionTracker::SaveCurrentItems (const QModelIndexList& rows)
	{
		CurrentItems_.clear ();
		for (const auto& row : rows)
			CurrentItems_ << row.data (IItemsModel::ItemRole::ItemId).value<IDType_t> ();
	}

	void ItemSelectionTracker::MarkRowAsRead (const QModelIndex& row)
	{
		ReadMarkTimer_.stop ();

		if (TapeMode_ || !row.isValid () || row.data (IItemsModel::ItemRole::IsRead).toBool ())
			return;

		const auto timeout = XmlSettingsManager::Instance ().property ("MarkAsReadTimeout").toInt ();
		ReadMarkTimer_.start (std::chrono::seconds { timeout });
	}
}

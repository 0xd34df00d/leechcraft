/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemslistmodel.h"
#include <algorithm>
#include <QApplication>
#include <QPalette>
#include <QTextDocument>
#include <QtDebug>
#include <interfaces/core/iiconthememanager.h>
#include "components/parsers/utils.h"
#include "storagebackendmanager.h"
#include "tooltipbuilder.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Aggregator
{
	ItemsListModel::ItemsListModel (IIconThemeManager *itm, QObject *parent)
	: QAbstractItemModel (parent)
	, StarredIcon_ (itm->GetIcon ("mail-mark-important"))
	, UnreadIcon_ (itm->GetIcon ("mail-mark-unread"))
	, ReadIcon_ (itm->GetIcon ("mail-mark-read"))
	{
		ItemHeaders_ << tr ("Name") << tr ("Date");

		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::channelRemoved,
				[this] (IDType_t id)
				{
					if (id == CurrentChannel_)
						Reset (IDNotFound);
				});
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::feedRemoved,
				[this] (IDType_t feedId)
				{
					if (CurrentChannel_ == IDNotFound)
						return;

					auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
					if (feedId == sb->GetChannel (CurrentChannel_).FeedID_)
						Reset (IDNotFound);
				});

		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::itemsRemoved,
				this,
				&ItemsListModel::RemoveItems);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::itemDataUpdated,
				this,
				[this] (const Item& item)
				{
					if (item.ChannelID_ == CurrentChannel_)
						ItemDataUpdated (item);
				});
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::itemReadStatusUpdated,
				this,
				&ItemsListModel::HandleItemReadStatusUpdated);
	}

	QAbstractItemModel& ItemsListModel::GetQModel ()
	{
		return *this;
	}

	const IDType_t& ItemsListModel::GetCurrentChannel () const
	{
		return CurrentChannel_;
	}

	const ItemShort& ItemsListModel::GetItem (const QModelIndex& index) const
	{
		return CurrentItems_ [index.row ()];
	}

	const items_shorts_t& ItemsListModel::GetAllItems () const
	{
		return CurrentItems_;
	}

	void ItemsListModel::Reset (IDType_t channel)
	{
		beginResetModel ();

		CurrentChannel_ = channel;
		CurrentItems_.clear ();

		if (channel != IDNotFound)
			CurrentItems_ = GetSB ()->GetItems (channel);

		endResetModel ();
	}

	void ItemsListModel::Reset (const QList<IDType_t>& items)
	{
		beginResetModel ();

		CurrentChannel_ = IDNotFound;
		CurrentItems_.clear ();

		const auto& sb = GetSB ();
		for (const IDType_t& itemId : items)
			if (const auto& item = sb->GetItem (itemId))
				CurrentItems_.push_back (item->ToShort ());

		endResetModel ();
	}

	void ItemsListModel::RemoveItems (const QSet<IDType_t>& ids)
	{
		int remainingCount = ids.size ();

		for (auto i = CurrentItems_.begin (); i != CurrentItems_.end () && remainingCount; )
		{
			if (!ids.contains (i->ItemID_))
			{
				++i;
				continue;
			}

			const size_t dist = std::distance (CurrentItems_.begin (), i);
			beginRemoveRows ({}, dist, dist);

			i = CurrentItems_.erase (i);
			--remainingCount;

			endRemoveRows ();
		}
	}

	void ItemsListModel::ItemDataUpdated (const Item& item)
	{
		const auto& is = item.ToShort ();

		const auto pos = std::find_if (CurrentItems_.begin (), CurrentItems_.end (),
				[&item] (const ItemShort& itemShort) { return item.ItemID_ == itemShort.ItemID_; });

		// Item is new
		if (pos == CurrentItems_.end ())
		{
			auto insertPos = std::find_if (CurrentItems_.begin (), CurrentItems_.end (),
						[&item] (const ItemShort& is) { return item.PubDate_ > is.PubDate_; });

			int shift = std::distance (CurrentItems_.begin (), insertPos);

			beginInsertRows (QModelIndex (), shift, shift);
			CurrentItems_.insert (insertPos, is);
			endInsertRows ();
		}
		// Item exists already
		else
		{
			*pos = is;

			int distance = std::distance (CurrentItems_.begin (), pos);
			emit dataChanged (index (distance, 0), index (distance, 1));
		}
	}

	int ItemsListModel::columnCount (const QModelIndex&) const
	{
		return ItemHeaders_.size ();
	}

	namespace
	{
		QVariant GetItemDisplay (const ItemShort& item, int column)
		{
			switch (column)
			{
			case 0:
			{
				auto title = item.Title_;
				auto pos = 0;
				while ((pos = title.indexOf ('<', pos)) != -1)
				{
					auto end = title.indexOf ('>', pos);
					if (end > 0)
						title.remove (pos, end - pos + 1);
					else
						break;
				}

				return Parsers::UnescapeHTML (std::move (title));
			}
			case 1:
				return item.PubDate_;
			}

			return {};
		}

		QVariant GetItemForeground (const ItemShort& item)
		{
			auto& xsm = *XmlSettingsManager::Instance ();
			bool palette = xsm.property ("UsePaletteColors").toBool ();
			if (item.Unread_)
			{
				if (xsm.property ("UnreadCustomColor").toBool ())
					return xsm.property ("UnreadItemsColor").value<QColor> ();
				else
					return palette ?
						QApplication::palette ().link ().color () :
						QVariant {};
			}
			else
				return palette ?
					QApplication::palette ().linkVisited ().color () :
					QVariant {};
		}

		QVariant GetItemBackground ()
		{
			const auto& p = QApplication::palette ();
			QLinearGradient grad { 0, 0, 0, 10 };
			grad.setColorAt (0, p.color (QPalette::AlternateBase));
			grad.setColorAt (1, p.color (QPalette::Base));
			return QBrush { grad };
		}
	}

	QVariant ItemsListModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid () || index.row () >= rowCount ())
			return {};

		const auto& item = CurrentItems_ [index.row ()];
		if (role == Qt::DisplayRole)
			return GetItemDisplay (item, index.column ());
		else if (role == Qt::ForegroundRole)
			return GetItemForeground (item);
		else if (role == Qt::FontRole &&
				item.Unread_)
			return XmlSettingsManager::Instance ()->property ("UnreadItemsFont");
		else if (role == Qt::ToolTipRole &&
				XmlSettingsManager::Instance ()->property ("ShowItemsTooltips").toBool ())
		{
			const auto& maybeItem = GetSB ()->GetItem (item.ItemID_);
			if (!maybeItem)
				return {};

			const auto& fullItem = *maybeItem;
			return TooltipBuilder { fullItem.Title_ }
					.Add (tr ("Author"), fullItem.Author_)
					.Add (tr ("Categories"), fullItem.Categories_.join ("; "))
					.Add (tr ("%n comment(s)", "", fullItem.NumComments_), fullItem.NumComments_)
					.Add (tr ("%n enclosure(s)", "", fullItem.Enclosures_.size ()), fullItem.Enclosures_.size ())
					.Add (tr ("%n MediaRSS entry(s)", "", fullItem.MRSSEntries_.size ()), fullItem.MRSSEntries_.size ())
					.Add (tr ("RSS with comments is available"), fullItem.CommentsLink_.size ())
					.AddHtml ("<hr/>" + fullItem.Description_)
					.GetTooltip ();
		}
		else if (role == Qt::BackgroundRole)
			return GetItemBackground ();
		else if (role == Qt::DecorationRole)
		{
			if (index.column ())
				return {};

			if (GetSB ()->GetItemTags (item.ItemID_).contains ("_important"))
				return StarredIcon_;

			return item.Unread_ ? UnreadIcon_ : ReadIcon_;
		}
		else if (role == ItemRole::IsRead)
			return !item.Unread_;
		else if (role == ItemRole::ItemId)
			return item.ItemID_;
		else if (role == ItemRole::ItemShortDescr)
			return QVariant::fromValue (item);
		else if (role == ItemRole::ItemCategories)
			return item.Categories_;
		else if (role == ItemRole::ItemImportant)
			return GetSB ()->GetItemTags (item.ItemID_).contains ("_important");
		else if (role == ItemRole::FullItem)
		{
			if (const auto maybeItem = GetSB ()->GetItem (item.ItemID_))
				return QVariant::fromValue (*maybeItem);
			else
				return {};
		}
		else
			return {};
	}

	Qt::ItemFlags ItemsListModel::flags (const QModelIndex&) const
	{
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}

	QVariant ItemsListModel::headerData (int column, Qt::Orientation orient, int role) const
	{
		if (orient == Qt::Horizontal && role == Qt::DisplayRole)
			return ItemHeaders_.at (column);
		else
			return {};
	}

	QModelIndex ItemsListModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return {};

		return createIndex (row, column);
	}

	QModelIndex ItemsListModel::parent (const QModelIndex&) const
	{
		return {};
	}

	int ItemsListModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : CurrentItems_.size ();
	}

	StorageBackend_ptr ItemsListModel::GetSB () const
	{
		if (!SB_.hasLocalData ())
			SB_.setLocalData (StorageBackendManager::Instance ().MakeStorageBackendForThread ());
		return SB_.localData ();
	}

	void ItemsListModel::HandleItemReadStatusUpdated (IDType_t channelId, IDType_t itemId, bool unread)
	{
		if (channelId != CurrentChannel_)
			return;

		const auto pos = std::find_if (CurrentItems_.begin (), CurrentItems_.end (),
				[&itemId] (const ItemShort& itemShort) { return itemShort.ItemID_ == itemId; });
		if (pos == CurrentItems_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find"
					<< channelId
					<< itemId;
			return;
		}

		pos->Unread_ = unread;

		int distance = std::distance (CurrentItems_.begin (), pos);
		emit dataChanged (index (distance, 0), index (distance, 1));
	}
}
}

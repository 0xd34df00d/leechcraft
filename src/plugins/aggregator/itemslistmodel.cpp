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
#include <util/sll/qtutil.h>
#include <interfaces/core/iiconthememanager.h>
#include "components/parsers/utils.h"
#include "storagebackendmanager.h"
#include "tooltipbuilder.h"
#include "xmlsettingsmanager.h"

namespace LC::Aggregator
{
	ItemsListModel::ItemsListModel (IIconThemeManager *itm, QObject *parent)
	: QAbstractItemModel { parent }
	, ItemHeaders_ { tr ("Name"), tr ("Date") }
	, StarredIcon_ { itm->GetIcon ("mail-mark-important") }
	, UnreadIcon_ { itm->GetIcon ("mail-mark-unread") }
	, ReadIcon_ { itm->GetIcon ("mail-mark-read") }
	{
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::channelRemoved,
				this,
				&ItemsListModel::RemoveChannel);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::feedRemoved,
				this,
				&ItemsListModel::RemoveFeed);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::itemsRemoved,
				this,
				&ItemsListModel::RemoveItems);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::itemDataUpdated,
				this,
				[this] (const Item& item)
				{
					if (CurrentChannels_.isEmpty () || CurrentChannels_.contains (item.ChannelID_))
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

	const ItemShort& ItemsListModel::GetItem (const QModelIndex& index) const
	{
		return CurrentItems_ [index.row ()];
	}

	const items_shorts_t& ItemsListModel::GetAllItems () const
	{
		return CurrentItems_;
	}

	void ItemsListModel::SetChannels (const QVector<IDType_t>& channels)
	{
		beginResetModel ();

		CurrentChannels_ = channels;

		CurrentItems_.clear ();

		for (const auto channel : channels)
			CurrentItems_ += GetSB ()->GetItems (channel);

		endResetModel ();
	}

	QList<QModelIndex> ItemsListModel::FindItems (const QSet<IDType_t>& ids) const
	{
		QList<QModelIndex> result;
		result.reserve (ids.size ());
		for (int i = 0; i < CurrentItems_.size (); ++i)
			if (ids.contains (CurrentItems_ [i].ItemID_))
			{
				result << index (i, 0);
				if (result.size () == ids.size ())
					return result;
			}

		return result;
	}

	void ItemsListModel::RemoveItems (const QSet<IDType_t>& ids)
	{
		auto remainingCount = ids.size ();

		for (auto i = CurrentItems_.begin (); i != CurrentItems_.end () && remainingCount; )
		{
			if (!ids.contains (i->ItemID_))
			{
				++i;
				continue;
			}

			const auto dist = i - CurrentItems_.begin ();
			beginRemoveRows ({}, dist, dist);

			i = CurrentItems_.erase (i);
			--remainingCount;

			endRemoveRows ();
		}
	}

	void ItemsListModel::RemoveChannel (IDType_t channelId)
	{
		RemoveChunked ([channelId] (const ItemShort& item) { return item.ChannelID_ == channelId; });
	}

	void ItemsListModel::RemoveFeed (IDType_t feedId)
	{
		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		QSet<IDType_t> channelIds;
		for (const auto& chan : sb->GetChannels (feedId))
			channelIds << chan.ChannelID_;

		RemoveChunked ([&channelIds] (const ItemShort& item) { return channelIds.contains (item.ChannelID_); });
	}

	template<typename F>
	void ItemsListModel::RemoveChunked (F&& filter)
	{
		auto pos = CurrentItems_.begin ();

		while (true)
		{
			pos = std::find_if (pos, CurrentItems_.end (), filter);
			if (pos == CurrentItems_.end ())
				break;

			const auto next = std::find_if (pos + 1, CurrentItems_.end (), std::not_fn (filter));

			beginRemoveRows ({}, pos - CurrentItems_.begin (), next - CurrentItems_.begin ());
			pos = CurrentItems_.erase (pos, next);
			endRemoveRows ();
		}
	}

	void ItemsListModel::ItemDataUpdated (const Item& item)
	{
		auto is = item.ToShort ();

		const auto pos = std::ranges::find_if (CurrentItems_,
				[&item] (const ItemShort& itemShort) { return item.ItemID_ == itemShort.ItemID_; });

		// Item is new
		if (pos == CurrentItems_.end ())
		{
			const auto row = CurrentItems_.size ();
			beginInsertRows ({}, row, row);
			CurrentItems_.push_back (std::move (is));
			endInsertRows ();
		}
		// Item exists already
		else
		{
			*pos = std::move (is);

			const auto distance = pos - CurrentItems_.begin ();
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
				return Parsers::UnescapeHTML (QString { item.Title_ });
			case 1:
				return item.PubDate_;
			default:
				return {};
			}
		}

		QVariant GetItemForeground (const ItemShort& item)
		{
			auto& xsm = XmlSettingsManager::Instance ();
			const bool palette = xsm.property ("UsePaletteColors").toBool ();
			if (item.Unread_)
			{
				if (xsm.property ("UnreadCustomColor").toBool ())
					return xsm.property ("UnreadItemsColor").value<QColor> ();
				return palette ?
					QApplication::palette ().link ().color () :
					QVariant {};
			}

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

		QString MakeItemTooltip (const Item& item)
		{
			return TooltipBuilder { item.Title_ }
					.Add (ItemsListModel::tr ("Author"), item.Author_)
					.Add (ItemsListModel::tr ("Categories"), item.Categories_.join ("; "_ql))
					.Add (ItemsListModel::tr ("%n comment(s)", "", item.NumComments_), item.NumComments_)
					.Add (ItemsListModel::tr ("%n enclosure(s)", "", item.Enclosures_.size ()), item.Enclosures_.size ())
					.Add (ItemsListModel::tr ("%n MediaRSS entry(s)", "", item.MRSSEntries_.size ()), item.MRSSEntries_.size ())
					.Add (ItemsListModel::tr ("RSS with comments is available"), item.CommentsLink_.size ())
					.AddHtml ("<hr/>" + item.Description_)
					.GetTooltip ();
		}
	}

	QVariant ItemsListModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid () || index.row () >= rowCount ())
			return {};

		const auto& item = CurrentItems_ [index.row ()];
		switch (role)
		{
		case Qt::DisplayRole:
			return GetItemDisplay (item, index.column ());
		case Qt::ForegroundRole:
			return GetItemForeground (item);
		case Qt::FontRole:
			return item.Unread_ ?
					XmlSettingsManager::Instance ().property ("UnreadItemsFont") :
					QVariant {};
		case Qt::ToolTipRole:
			return GetSB ()->GetItem (item.ItemID_)
					.transform (&MakeItemTooltip)
					.value_or (QString {});
		case Qt::BackgroundRole:
			return GetItemBackground ();
		case Qt::DecorationRole:
		{
			if (index.column ())
				return {};

			if (GetSB ()->GetItemTags (item.ItemID_).contains ("_important"))
				return StarredIcon_;

			return item.Unread_ ? UnreadIcon_ : ReadIcon_;
		}
		case ItemRole::IsRead:
			return !item.Unread_;
		case ItemRole::ItemId:
			return item.ItemID_;
		case ItemRole::ItemShortDescr:
			return QVariant::fromValue (item);
		case ItemRole::ItemCategories:
			return item.Categories_;
		case ItemRole::ItemImportant:
			return GetSB ()->GetItemTags (item.ItemID_).contains ("_important");
		case ItemRole::ItemChannelId:
			return item.ChannelID_;
		case ItemRole::FullItem:
			return GetSB ()->GetItem (item.ItemID_)
					.transform ([] (auto&& value) { return QVariant::fromValue (std::move (value)); })
					.value_or (QVariant {});
		default:
			break;
		}

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
		if (!CurrentChannels_.isEmpty () && !CurrentChannels_.contains (channelId))
			return;

		const auto pos = std::ranges::find_if (CurrentItems_,
				[&itemId] (const ItemShort& itemShort) { return itemShort.ItemID_ == itemId; });
		if (pos == CurrentItems_.end ())
			return;

		pos->Unread_ = unread;

		const auto distance = pos - CurrentItems_.begin ();
		emit dataChanged (index (distance, 0), index (distance, 1));
	}
}

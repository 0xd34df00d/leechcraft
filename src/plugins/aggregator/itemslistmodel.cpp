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
#include "xmlsettingsmanager.h"
#include "storagebackendmanager.h"

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
				&ItemsListModel::HandleItemsRemoved);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::itemDataUpdated,
				this,
				&ItemsListModel::HandleItemDataUpdated);
		connect (&StorageBackendManager::Instance (),
				&StorageBackendManager::itemReadStatusUpdated,
				this,
				&ItemsListModel::HandleItemReadStatusUpdated);
	}

	int ItemsListModel::GetSelectedRow () const
	{
		return CurrentRow_;
	}

	const IDType_t& ItemsListModel::GetCurrentChannel () const
	{
		return CurrentChannel_;
	}

	void ItemsListModel::Selected (const QModelIndex& index)
	{
		CurrentRow_ = index.row ();
		if (!index.isValid ())
			return;

		const auto& item = CurrentItems_ [CurrentRow_];
		if (!item.Unread_)
			return;

		GetSB ()->SetItemUnread (item.ItemID_, false);
	}

	const ItemShort& ItemsListModel::GetItem (const QModelIndex& index) const
	{
		return CurrentItems_ [index.row ()];
	}

	const items_shorts_t& ItemsListModel::GetAllItems () const
	{
		return CurrentItems_;
	}

	bool ItemsListModel::IsItemRead (int item) const
	{
		return !CurrentItems_ [item].Unread_;
	}

	QStringList ItemsListModel::GetCategories (int item) const
	{
		return CurrentItems_ [item].Categories_;
	}

	void ItemsListModel::Reset (IDType_t channel)
	{
		beginResetModel ();

		CurrentChannel_ = channel;
		CurrentRow_ = -1;
		CurrentItems_.clear ();

		if (channel != IDNotFound)
			CurrentItems_ = GetSB ()->GetItems (channel);

		endResetModel ();
	}

	void ItemsListModel::Reset (const QList<IDType_t>& items)
	{
		beginResetModel ();

		CurrentChannel_ = IDNotFound;
		CurrentRow_ = -1;
		CurrentItems_.clear ();

		const auto& sb = GetSB ();
		for (const IDType_t& itemId : items)
			if (const auto& item = sb->GetItem (itemId))
				CurrentItems_.push_back (item->ToShort ());

		endResetModel ();
	}

	void ItemsListModel::RemoveItems (const QSet<IDType_t>& ids)
	{
		if (ids.isEmpty ())
			return;

		const bool shouldReset = ids.size () > 10;

		if (shouldReset)
			beginResetModel ();

		int remainingCount = ids.size ();

		for (auto i = CurrentItems_.begin ();
				i != CurrentItems_.end () && remainingCount; )
		{
			if (!ids.contains (i->ItemID_))
			{
				++i;
				continue;
			}

			if (!shouldReset)
			{
				const size_t dist = std::distance (CurrentItems_.begin (), i);
				beginRemoveRows (QModelIndex (), dist, dist);
			}

			i = CurrentItems_.erase (i);
			--remainingCount;

			if (!shouldReset)
			{
				endRemoveRows ();
				qApp->processEvents (QEventLoop::ExcludeUserInputEvents);
			}
		}

		if (shouldReset)
			endResetModel ();
	}

	void ItemsListModel::ItemDataUpdated (const Item& item)
	{
		const auto& is = item.ToShort ();

		const auto pos = std::find_if (CurrentItems_.begin (), CurrentItems_.end (),
				[&item] (const ItemShort& itemShort)
				{
					return item.ItemID_ == itemShort.ItemID_ ||
							(item.Title_ == itemShort.Title_ && item.Link_ == itemShort.URL_);
				});

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
		void RemoveTag (const QString& name, QString& str)
		{
			int startPos = 0;
			while ((startPos = str.indexOf ("<" + name, startPos, Qt::CaseInsensitive)) >= 0)
			{
				const int end = str.indexOf ('>', startPos);
				if (end < 0)
					return;

				str.remove (startPos, end - startPos + 1);
			}
		}

		void RemovePair (const QString& name, QString& str)
		{
			RemoveTag (name, str);
			RemoveTag ('/' + name, str);
		}
	}

	QVariant ItemsListModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid () || index.row () >= rowCount ())
			return QVariant ();

		if (role == Qt::DisplayRole)
		{
			switch (index.column ())
			{
				case 0:
					{
						auto title = CurrentItems_ [index.row ()].Title_;
						auto pos = 0;
						while ((pos = title.indexOf ('<', pos)) != -1)
						{
							auto end = title.indexOf ('>', pos);
							if (end > 0)
								title.remove (pos, end - pos + 1);
							else
								break;
						}

						title.replace ("&laquo;", QString::fromUtf8 ("«"));
						title.replace ("&raquo;", QString::fromUtf8 ("»"));
						title.replace ("&quot;", "\"");
						title.replace ("&ndash;", "-");
						title.replace ("&mdash;", QString::fromUtf8 ("—"));

						return title;
					}
				case 1:
					return CurrentItems_ [index.row ()].PubDate_;
				default:
					return QVariant ();
			}
		}
		//Color mark an items as read/unread
		else if (role == Qt::ForegroundRole)
		{
			bool palette = XmlSettingsManager::Instance ()->
					property ("UsePaletteColors").toBool ();
			if (CurrentItems_ [index.row ()].Unread_)
			{
				if (XmlSettingsManager::Instance ()->
						property ("UnreadCustomColor").toBool ())
					return XmlSettingsManager::Instance ()->
							property ("UnreadItemsColor").value<QColor> ();
				else
					return palette ?
						QApplication::palette ().link ().color () :
						QVariant ();
			}
			else
				return palette ?
					QApplication::palette ().linkVisited ().color () :
					QVariant ();
		}
		else if (role == Qt::FontRole &&
				CurrentItems_ [index.row ()].Unread_)
			return XmlSettingsManager::Instance ()->
				property ("UnreadItemsFont");
		else if (role == Qt::ToolTipRole &&
				XmlSettingsManager::Instance ()->property ("ShowItemsTooltips").toBool ())
		{
			IDType_t id = CurrentItems_ [index.row ()].ItemID_;
			const auto& maybeItem = GetSB ()->GetItem (id);
			if (!maybeItem)
				return {};

			const auto& item = *maybeItem;
			QString result = QString ("<qt><strong>%1</strong><br />").arg (item.Title_);
			if (item.Author_.size ())
			{
				result += tr ("<b>Author</b>: %1").arg (item.Author_);
				result += "<br />";
			}
			if (item.Categories_.size ())
			{
				result += tr ("<b>Categories</b>: %1").arg (item.Categories_.join ("; "));
				result += "<br />";
			}
			if (item.NumComments_ > 0)
			{
				result += tr ("%n comment(s)", "", item.NumComments_);
				result += "<br />";
			}
			if (item.Enclosures_.size () > 0)
			{
				result += tr ("%n enclosure(s)", "", item.Enclosures_.size ());
				result += "<br />";
			}
			if (item.MRSSEntries_.size () > 0)
			{
				result += tr ("%n MediaRSS entry(s)", "", item.MRSSEntries_.size ());
				result += "<br />";
			}
			if (item.CommentsLink_.size ())
			{
				result += tr ("RSS with comments is available");
				result += "<br />";
			}
			result += "<br />";

			const int maxDescriptionSize = 1000;
			auto descr = item.Description_;
			RemoveTag ("img", descr);
			RemovePair ("font", descr);
			RemovePair ("span", descr);
			RemovePair ("p", descr);
			RemovePair ("div", descr);
			for (auto i : { 1, 2, 3, 4, 5, 6 })
				RemovePair ("h" + QString::number (i), descr);
			result += descr.left (maxDescriptionSize);
			if (descr.size () > maxDescriptionSize)
				result += "...";

			return result;
		}
		else if (role == Qt::BackgroundRole)
		{
			const QPalette& p = QApplication::palette ();
			QLinearGradient grad (0, 0, 0, 10);
			grad.setColorAt (0, p.color (QPalette::AlternateBase));
			grad.setColorAt (1, p.color (QPalette::Base));
			return QBrush (grad);
		}
		else if (role == Qt::DecorationRole)
		{
			if (index.column ())
				return QVariant ();

			const auto& item = CurrentItems_ [index.row ()];
			if (GetSB ()->GetItemTags (item.ItemID_).contains ("_important"))
				return StarredIcon_;

			return item.Unread_ ? UnreadIcon_ : ReadIcon_;
		}
		else if (role == ItemRole::IsRead)
			return !CurrentItems_ [index.row ()].Unread_;
		else if (role == ItemRole::ItemId)
			return CurrentItems_ [index.row ()].ItemID_;
		else if (role == ItemRole::ItemShortDescr)
			return QVariant::fromValue (CurrentItems_ [index.row ()]);
		else
			return QVariant ();
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
			return QVariant ();
	}

	QModelIndex ItemsListModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex ItemsListModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
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

	void ItemsListModel::HandleItemsRemoved (const QSet<IDType_t>& items)
	{
		RemoveItems (items);
	}

	void ItemsListModel::HandleItemDataUpdated (const Item& item)
	{
		if (item.ChannelID_ != CurrentChannel_)
			return;

		ItemDataUpdated (item);
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

	void ItemsListModel::reset (IDType_t channelId)
	{
		Reset (channelId);
	}

	void ItemsListModel::selected (const QModelIndex& index)
	{
		Selected (index);
	}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "itemslistmodel.h"
#include <QApplication>
#include <QPalette>
#include <QTextDocument>
#include <QtDebug>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			ItemsListModel::ItemsListModel (QObject *parent)
			: QAbstractItemModel (parent)
			, CurrentRow_ (-1)
			, MayBeRichText_ (false)
			{
				ItemHeaders_ << tr ("Name") << tr ("Date");
			}
			
			int ItemsListModel::GetSelectedRow () const
			{
				return CurrentRow_;
			}
			
			const QPair<QString, QString>& ItemsListModel::GetHash () const
			{
				return CurrentChannelHash_;
			}
			
			void ItemsListModel::SetHash (const QPair<QString, QString>& hash)
			{
				CurrentChannelHash_ = hash;
				reset ();
			}
			
			void ItemsListModel::Selected (const QModelIndex& index)
			{
				CurrentRow_ = index.row ();
				ItemShort item = CurrentItems_ [index.row ()];
				item.Unread_ = false;
				Core::Instance ().GetStorageBackend ()->UpdateItem (item,
						CurrentChannelHash_.first, CurrentChannelHash_.second);
			}
			
			void ItemsListModel::MarkItemAsUnread (const QModelIndex& i)
			{
				ItemShort is = CurrentItems_ [i.row ()];
				is.Unread_ = true;
				Core::Instance ().GetStorageBackend ()->UpdateItem (is,
						CurrentChannelHash_.first, CurrentChannelHash_.second);
			}
			
			const ItemShort& ItemsListModel::GetItem (const QModelIndex& index) const
			{
				return CurrentItems_ [index.row ()];
			}
			
			bool ItemsListModel::IsItemRead (int item) const
			{
				return !CurrentItems_ [item].Unread_;
			}
			
			QStringList ItemsListModel::GetCategories (int item) const
			{
				return CurrentItems_ [item].Categories_;
			}
			
			void ItemsListModel::Reset (const QPair<QString, QString>& hash)
			{
				CurrentChannelHash_ = hash;
				CurrentRow_ = -1;
				CurrentItems_.clear ();
				Core::Instance ().GetStorageBackend ()->
					GetItems (CurrentItems_, hash.first + hash.second);
				if (CurrentItems_.size ())
					MayBeRichText_ = Qt::mightBeRichText (CurrentItems_.at (0).Title_);
				reset ();
			}
			
			namespace
			{
				struct FindEarlierDate
				{
					QDateTime Pattern_;
			
					FindEarlierDate (const QDateTime& pattern)
					: Pattern_ (pattern)
					{
					}
			
					bool operator() (const ItemShort& is)
					{
						return Pattern_ > is.PubDate_;
					}
				};
			};
			
			void ItemsListModel::ItemDataUpdated (Item_ptr item)
			{
				ItemShort is = item->ToShort ();
			
				items_shorts_t::iterator pos = CurrentItems_.end ();
			
				for (items_shorts_t::iterator i = CurrentItems_.begin (),
						end = CurrentItems_.end (); i != end; ++i)
					if (is.Title_ == i->Title_ &&
							is.URL_ == i->URL_)
					{
						pos = i;
						break;
					}
			
				// Item is new
				if (pos == CurrentItems_.end ())
				{
					items_shorts_t::iterator insertPos =
						std::find_if (CurrentItems_.begin (), CurrentItems_.end (),
								FindEarlierDate (item->PubDate_));
			
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
								QString title = CurrentItems_ [index.row ()].Title_;
								if (MayBeRichText_ &&
										Qt::mightBeRichText (title))
								{
									QTextDocument doc;
									doc.setHtml (title);
									title = doc.toPlainText ();
								}
								return title;
							}
						case 1:
							return CurrentItems_ [index.row ()].PubDate_;
						default:
							return QVariant ();
					}
				}
				else if (role == Qt::ForegroundRole)
					if (CurrentItems_ [index.row ()].Unread_ &&
							XmlSettingsManager::Instance ()->
							property ("UnreadCustomColor").toBool ())
						return XmlSettingsManager::Instance ()->
							property ("UnreadItemsColor").value<QColor> ();
					else
						return QVariant ();
				else if (role == Qt::FontRole &&
						CurrentItems_ [index.row ()].Unread_)
					return XmlSettingsManager::Instance ()->
						property ("UnreadItemsFont");
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
		};
	};
};


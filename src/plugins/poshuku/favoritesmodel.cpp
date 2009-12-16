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

#include "favoritesmodel.h"
#include <algorithm>
#include <QTimer>
#include <QtDebug>
#include "filtermodel.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			bool FavoritesModel::FavoritesItem::operator== (const FavoritesModel::FavoritesItem& item) const
			{
				return Title_ == item.Title_ &&
					URL_ == item.URL_ &&
					Tags_ == item.Tags_;
			}
			
			FavoritesModel::FavoritesModel (QObject *parent)
			: QAbstractItemModel (parent)
			{
				ItemHeaders_ << tr ("Title")
					<< tr ("URL")
					<< tr ("Tags");
				QTimer::singleShot (0, this, SLOT (loadData ()));
			}
			
			FavoritesModel::~FavoritesModel ()
			{
			}
			
			int FavoritesModel::columnCount (const QModelIndex&) const
			{
				return ItemHeaders_.size ();
			}
			
			QVariant FavoritesModel::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();
			
				switch (role)
				{
					case Qt::DisplayRole:
						switch (index.column ())
						{
							case ColumnTitle:
								return Items_ [index.row ()].Title_;
							case ColumnURL:
								return Items_ [index.row ()].URL_;
							case ColumnTags:
								return Core::Instance ().GetProxy ()->
									GetTagsManager ()->Join (GetVisibleTags (index.row ()));
							default:
								return QVariant ();
						}
					case Qt::DecorationRole:
						if (index.column () == ColumnTitle)
							return Core::Instance ()
								.GetIcon (Items_ [index.row ()].URL_);
						else
							return QVariant ();
					case Qt::ToolTipRole:
						return CheckResults_ [Items_ [index.row ()].URL_];
					case RoleTags:
						return Items_ [index.row ()].Tags_;
					default:
						return QVariant ();
				}
			}
			
			Qt::ItemFlags FavoritesModel::flags (const QModelIndex& index) const
			{
				Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
				if (index.column () == ColumnTags)
					result |= Qt::ItemIsEditable;
				return result;
			}
			
			QVariant FavoritesModel::headerData (int column, Qt::Orientation orient,
					int role) const
			{
				if (orient == Qt::Horizontal && role == Qt::DisplayRole)
					return ItemHeaders_.at (column);
				else
					return QVariant ();
			}
			
			QModelIndex FavoritesModel::index (int row, int column,
					const QModelIndex& parent) const
			{
				if (!hasIndex (row, column, parent))
					return QModelIndex ();
			
				return createIndex (row, column);
			}
			
			QModelIndex FavoritesModel::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int FavoritesModel::rowCount (const QModelIndex& index) const
			{
				return index.isValid () ? 0 : Items_.size ();
			}
			
			/** The passed value is a string list with user-visible tags.
			 */
			bool FavoritesModel::setData (const QModelIndex& index,
					const QVariant& value, int)
			{
				switch (index.column ())
				{
					case ColumnTags:
						{
							QStringList userTags = value.toStringList ();
							Items_ [index.row ()].Tags_.clear ();
							Q_FOREACH (QString ut, userTags) 
								Items_ [index.row ()].Tags_.append (Core::Instance ().GetProxy ()->
										GetTagsManager ()->GetID (ut));
							Core::Instance ().GetStorageBackend ()->
								UpdateFavorites (Items_ [index.row ()]);
							return true;
						}
					case ColumnTitle:
						{
							QString title = value.toString ();
							Items_ [index.row ()].Title_ = title;
							Core::Instance ().GetStorageBackend ()->
								UpdateFavorites (Items_ [index.row ()]);
							return true;
						}
					case ColumnURL:
						{
							return true;
						}
					default:
						return false;
				}
			}
			
			bool FavoritesModel::AddItem (const QString& title, const QString& url,
				   const QStringList& visibleTags)
			{
				QStringList tags;
				Q_FOREACH (QString vt, visibleTags)
					tags << Core::Instance ().GetProxy ()->GetTagsManager ()->GetID (vt);
				FavoritesItem item =
				{
					title,
					url,
					tags
				};
			
				try
				{
					Core::Instance ().GetStorageBackend ()->AddToFavorites (item);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					emit error (tr ("Failed to add<br />%1<br />to Favorites, seems "
								"like such title is already used.").arg (title));
					return false;
				}
				return true;
			}

			void FavoritesModel::ChangeURL (const QModelIndex& index,
					const QString& url)
			{
				FavoritesItem item = Items_.at (index.row ());
				if (item.URL_ == url)
					return;

				removeItem (index);
				item.URL_ = url;
				Core::Instance ().GetStorageBackend ()->AddToFavorites (item);
			}
			
			const FavoritesModel::items_t& FavoritesModel::GetItems () const
			{
				return Items_;
			}

			namespace
			{
				struct ItemFinder
				{
					const QString& URL_;
			
					ItemFinder (const QString& url)
					: URL_ (url)
					{
					}
			
					bool operator() (const FavoritesModel::FavoritesItem& item) const
					{
						return item.URL_ == URL_;
					}
				};
			};
			
			void FavoritesModel::SetCheckResults (const QMap<QString, QString>& res)
			{
				CheckResults_ = res;
			}
			
			QStringList FavoritesModel::GetVisibleTags (int index) const
			{
				QStringList user;
				Q_FOREACH (QString id, Items_ [index].Tags_)
					user.append (Core::Instance ().GetProxy ()->GetTagsManager ()->
							GetTag (id));
				return user;
			}
			
			void FavoritesModel::removeItem (const QModelIndex& index)
			{
				Core::Instance ().GetStorageBackend ()->
					RemoveFromFavorites (Items_ [index.row ()]);
			}
			
			void FavoritesModel::handleItemAdded (const FavoritesModel::FavoritesItem& item)
			{
				beginInsertRows (QModelIndex (), 0, 0);
				Items_.push_back (item);
				endInsertRows ();
			}
			
			void FavoritesModel::handleItemUpdated (const FavoritesModel::FavoritesItem& item)
			{
				items_t::iterator pos =
					std::find_if (Items_.begin (), Items_.end (), ItemFinder (item.URL_));
				if (pos == Items_.end ())
				{
					qWarning () << Q_FUNC_INFO << "not found updated item";
					return;
				}
			
				*pos = item;
			
				int n = std::distance (Items_.begin (), pos);
			
				emit dataChanged (index (n, 0), index (n, 2));
			}
			
			void FavoritesModel::handleItemRemoved (const FavoritesModel::FavoritesItem& item)
			{
				items_t::iterator pos =
					std::find (Items_.begin (), Items_.end (), item);
				if (pos == Items_.end ())
				{
					qWarning () << Q_FUNC_INFO << "not found removed item";
					return;
				}
			
				int n = std::distance (Items_.begin (), pos);
				beginRemoveRows (QModelIndex (), n, n);
				Items_.erase (pos);
				endRemoveRows ();
			}
			
			void FavoritesModel::loadData ()
			{
				items_t items;
				Core::Instance ().GetStorageBackend ()->LoadFavorites (items);
			
				if (!items.size ())
					return;
			
				beginInsertRows (QModelIndex (), 0, items.size () - 1);
				for (items_t::reverse_iterator i = items.rbegin (),
						end = items.rend (); i != end; ++i)
				{
					Q_FOREACH (QString tag, i->Tags_)
					{
						QString ut = Core::Instance ().GetProxy ()->
							GetTagsManager ()->GetTag (tag);
						if (ut.isEmpty ())
							i->Tags_.removeAll (tag);
					}

					Items_.push_back (*i);
				}
				endInsertRows ();
			}
		};
	};
};


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

#include "core.h"
#include <QtDebug>
#include "itemsfiltermodel.h"
#include "itemswidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			ItemsFilterModel::ItemsFilterModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			, HideRead_ (false)
			, ItemsWidget_ (0)
			, UnreadOnTop_ (XmlSettingsManager::Instance ()->
					property ("UnreadOnTop").toBool ())
			{
				setDynamicSortFilter (true);

				XmlSettingsManager::Instance ()->RegisterObject ("UnreadOnTop",
						this, "handleUnreadOnTopChanged");
			}
			
			ItemsFilterModel::~ItemsFilterModel ()
			{
			}

			void ItemsFilterModel::SetItemsWidget (ItemsWidget *w)
			{
				ItemsWidget_ = w;
			}
			
			void ItemsFilterModel::SetHideRead (bool hide)
			{
				HideRead_ = hide;
				invalidateFilter ();
			}
			
			bool ItemsFilterModel::filterAcceptsRow (int sourceRow,
					const QModelIndex& sourceParent) const
			{
				if (HideRead_ &&
						ItemsWidget_->IsItemReadNotCurrent (sourceRow))
					return false;

				if (!ItemCategories_.isEmpty ())
				{
					bool categoryFound = false;
					QStringList itemCategories =
						ItemsWidget_->GetItemCategories (sourceRow);
			
					if (!itemCategories.size ())
						categoryFound = true;
					else
						Q_FOREACH (QString cat, itemCategories)
							if (ItemCategories_.contains (cat))
							{
								categoryFound = true;
								break;
							}
			
					if (!categoryFound)
						return false;
				}
			
				return QSortFilterProxyModel::filterAcceptsRow (sourceRow,
						sourceParent);
			}
			
			bool ItemsFilterModel::lessThan (const QModelIndex& left,
					const QModelIndex& right) const
			{
				if (left.column () == 1 &&
						right.column () == 1 &&
						UnreadOnTop_ &&
						!HideRead_)
				{
					bool lr = ItemsWidget_->IsItemRead (left.row ());
					bool rr = ItemsWidget_->IsItemRead (right.row ());
					if (lr && !rr)
						return true;
					else if ((lr && rr) || (!lr && !rr))
						return QSortFilterProxyModel::lessThan (left, right);
					else
						return false;
				}
				return QSortFilterProxyModel::lessThan (left, right);
			}

			void ItemsFilterModel::categorySelectionChanged (const QStringList& categories)
			{
				ItemCategories_ = QSet<QString>::fromList (categories);
				invalidateFilter ();
			}

			void ItemsFilterModel::handleUnreadOnTopChanged ()
			{
				UnreadOnTop_ = XmlSettingsManager::Instance ()->
						property ("UnreadOnTop").toBool ();
				invalidateFilter ();
			}
		};
	};
};


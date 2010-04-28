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

#ifndef PLUGINS_AGGREGATOR_ITEMSFILTERMODEL_H
#define PLUGINS_AGGREGATOR_ITEMSFILTERMODEL_H
#include <QSortFilterProxyModel>
#include <QSet>
#include <QString>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemsWidget;

			class ItemsFilterModel : public QSortFilterProxyModel
			{
				Q_OBJECT

				bool HideRead_;
				bool UnreadOnTop_;
				QSet<QString> ItemCategories_;
				ItemsWidget *ItemsWidget_;
			public:
				ItemsFilterModel (QObject* = 0);
				virtual ~ItemsFilterModel ();

				void SetItemsWidget (ItemsWidget*);
				void SetHideRead (bool);
			protected:
				virtual bool filterAcceptsRow (int, const QModelIndex&) const;
				virtual bool lessThan (const QModelIndex&, const QModelIndex&) const;
			public slots:
				void categorySelectionChanged (const QStringList&);
				void handleUnreadOnTopChanged ();
			};
		};
	};
};

#endif


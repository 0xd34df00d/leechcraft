/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_ITEMSFILTERMODEL_H
#define PLUGINS_AGGREGATOR_ITEMSFILTERMODEL_H
#include <QSortFilterProxyModel>
#include <QSet>
#include <QString>
#include <interfaces/core/itagsmanager.h>
#include "common.h"

namespace LC
{
namespace Aggregator
{
	class ItemsWidget;

	class ItemsFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT

		bool HideRead_ = false;
		bool UnreadOnTop_ = false;
		QSet<QString> ItemCategories_;
		ItemsWidget *ItemsWidget_ = nullptr;
		QSet<IDType_t> TaggedItems_;
	public:
		ItemsFilterModel (QObject* = 0);

		void SetItemsWidget (ItemsWidget*);
		void SetHideRead (bool);
		void SetItemTags (QList<ITagsManager::tag_id>);
	protected:
		virtual bool filterAcceptsRow (int, const QModelIndex&) const;
		virtual bool lessThan (const QModelIndex&, const QModelIndex&) const;
	public slots:
		void categorySelectionChanged (const QStringList&);
	};
}
}

#endif

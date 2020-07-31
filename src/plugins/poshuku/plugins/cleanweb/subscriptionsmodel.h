/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include "filter.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	class SubscriptionsModel : public QAbstractItemModel
	{
		Q_OBJECT

		const QStringList HeaderLabels_;

		QList<Filter> Filters_;
	public:
		SubscriptionsModel (QObject* = nullptr);

		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int) const override;
		QVariant headerData (int, Qt::Orientation, int) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex&) const override;

		void SetInitialFilters (const QList<Filter>&);

		void AddFilter (const Filter&);
		void SetSubData (const SubscriptionData&);

		void RemoveFilter (const QString& filename);
		void RemoveFilter (const QModelIndex&);

		template<typename T, typename F>
		bool HasFilter (T val, F g) const
		{
			return std::any_of (Filters_.begin (), Filters_.end (),
					[&val, &g] (const Filter& f) { return g (f) == val; });
		}

		const QList<Filter>& GetAllFilters () const;
	private:
		void AddImpl (const Filter&);
		void RemoveFilter (int);

		void SaveSettings () const;
		void LoadSettings ();
		bool AssignSD (const SubscriptionData&);
	signals:
		void filtersListChanged ();
	};
}
}
}

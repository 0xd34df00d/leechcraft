/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>
#include <QStringList>
#include <util/models/rolenamesmixin.h>

namespace LC
{
namespace Launchy
{
	class ItemsSortFilterProxyModel final : public Util::RoleNamesMixin<QSortFilterProxyModel>
	{
		Q_OBJECT

		QStringList CategoryNames_;
		QString AppFilterText_;
	public:
		explicit ItemsSortFilterProxyModel (QAbstractItemModel*, QObject* = nullptr);

		QString GetAppFilterText () const;
		void SetAppFilterText (const QString&);
	protected:
		bool lessThan (const QModelIndex& left, const QModelIndex& right) const override;
		bool filterAcceptsRow (int, const QModelIndex&) const override;
	public slots:
		void setCategoryNames (const QStringList&);
	private slots:
		void invalidateFilterSlot ();
	};
}
}

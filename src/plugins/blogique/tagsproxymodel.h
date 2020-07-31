/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC
{
namespace Blogique
{
	class TagsProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT

		Q_PROPERTY (int count READ GetCount NOTIFY countChanged)
	public:
		explicit TagsProxyModel (QObject *parent = 0);

		bool filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const;
		bool lessThan (const QModelIndex& left, const QModelIndex& right) const;
		int GetCount () const;
		Q_INVOKABLE QString GetTagName (int index);

		void countUpdated ();
	signals:
		void countChanged ();
	};
}
}

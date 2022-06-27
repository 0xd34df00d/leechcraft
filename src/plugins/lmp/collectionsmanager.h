/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QModelIndex;
class QAbstractItemModel;
class QSortFilterProxyModel;

namespace LC::LMP
{
	class Player;

	class CollectionsManager : public QObject
	{
		QAbstractItemModel *Model_ = nullptr;
		QSortFilterProxyModel * const Sorter_;
	public:
		explicit CollectionsManager (QObject* = nullptr);

		void SetCollectionModel (QAbstractItemModel*);

		QAbstractItemModel* GetModel () const;

		void Enqueue (const QList<QModelIndex>&, Player*);
	};
}

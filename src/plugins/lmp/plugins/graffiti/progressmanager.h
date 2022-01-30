/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QHash>

class QStandardItemModel;
class QAbstractItemModel;
class QStandardItem;

namespace LC::LMP::Graffiti
{
	class CueSplitter;

	class ProgressManager : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::Graffiti::ProgressManager)

		QStandardItemModel *Model_;
		QHash<QObject*, QList<QStandardItem*>> TagsFetchObj2Row_;
		QHash<CueSplitter*, QList<QStandardItem*>> Splitter2Row_;
	public:
		explicit ProgressManager (QObject* = nullptr);

		QAbstractItemModel* GetModel () const;

		void HandleTagsFetch (int fetched, int total, QObject *obj);
		void HandleCueSplitter (CueSplitter*);
	};
}

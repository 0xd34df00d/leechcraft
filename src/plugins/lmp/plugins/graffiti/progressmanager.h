/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>

class QStandardItemModel;
class QAbstractItemModel;
class QStandardItem;

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	class CueSplitter;

	class ProgressManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		QHash<QObject*, QList<QStandardItem*>> TagsFetchObj2Row_;
		QHash<CueSplitter*, QList<QStandardItem*>> Splitter2Row_;
	public:
		ProgressManager (QObject* = 0);

		QAbstractItemModel* GetModel () const;
	public slots:
		void handleTagsFetch (int fetched, int total, QObject *obj);

		void handleCueSplitter (CueSplitter*);
		void handleSplitProgress (int, int, CueSplitter*);
		void handleSplitFinished (CueSplitter*);
	};
}
}
}

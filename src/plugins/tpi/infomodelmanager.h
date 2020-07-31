/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPersistentModelIndex>
#include <interfaces/core/icoreproxy.h>

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;

namespace LC
{
namespace TPI
{
	class InfoModelManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QStandardItemModel *Model_;

		QHash<QPersistentModelIndex, QStandardItem*> PIdx2Item_;
	public:
		InfoModelManager (ICoreProxy_ptr, QObject* = 0);

		QAbstractItemModel* GetModel () const;

		void SecondInit ();
	private:
		void ManageModel (QAbstractItemModel*);
		void HandleRows (QAbstractItemModel*, int, int);
		void HandleData (QAbstractItemModel*, int, int);
	private slots:
		void handleRowsInserted (const QModelIndex&, int, int);
		void handleRowsRemoved (const QModelIndex&, int, int);
		void handleDataChanged (const QModelIndex&, const QModelIndex&);
	};
}
}

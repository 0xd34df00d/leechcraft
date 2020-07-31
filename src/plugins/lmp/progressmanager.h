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

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace LMP
{
	class SyncManagerBase;

	class ProgressManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;

		typedef QHash<SyncManagerBase*, QList<QStandardItem*>> Syncer2Row_t;
		Syncer2Row_t TCRows_;
		Syncer2Row_t UpRows_;
	public:
		ProgressManager (QObject* = 0);

		QAbstractItemModel* GetModel () const;

		void AddSyncManager (SyncManagerBase*);
	private:
		void HandleWithHash (int, int, SyncManagerBase*,
				Syncer2Row_t&, const QString&, const QString&);
	private slots:
		void handleTCProgress (int, int, SyncManagerBase*);
		void handleUploadProgress (int, int, SyncManagerBase*);
	};
}
}

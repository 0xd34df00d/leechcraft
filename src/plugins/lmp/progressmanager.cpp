/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "progressmanager.h"
#include <QStandardItemModel>
#include <util/xpc/util.h>
#include <interfaces/ijobholder.h>
#include "sync/syncmanagerbase.h"

namespace LC
{
namespace LMP
{
	ProgressManager::ProgressManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
	}

	QAbstractItemModel* ProgressManager::GetModel () const
	{
		return Model_;
	}

	void ProgressManager::AddSyncManager (SyncManagerBase *syncManager)
	{
		connect (syncManager,
				SIGNAL (transcodingProgress (int, int, SyncManagerBase*)),
				this,
				SLOT (handleTCProgress (int, int, SyncManagerBase*)));
		connect (syncManager,
				SIGNAL (uploadProgress (int, int, SyncManagerBase*)),
				this,
				SLOT (handleUploadProgress (int, int, SyncManagerBase*)));
	}

	void ProgressManager::HandleWithHash (int done, int total,
			SyncManagerBase *syncer, Syncer2Row_t& hash, const QString& name, const QString& status)
	{
		if (!hash.contains (syncer))
		{
			if (done == total)
				return;

			const QList<QStandardItem*> row
			{
				new QStandardItem (name),
				new QStandardItem (status),
				new QStandardItem ()
			};
			auto item = row.at (JobHolderColumn::JobProgress);
			item->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
					CustomDataRoles::RoleJobHolderRow);

			hash [syncer] = row;
			Model_->appendRow (row);
		}

		const auto& row = hash [syncer];
		if (done == total)
		{
			Model_->removeRow (row.first ()->row ());
			hash.remove (syncer);
			return;
		}

		Util::SetJobHolderProgress (row, done, total, tr ("%1 of %2").arg (done).arg (total));
	}

	void ProgressManager::handleTCProgress (int done, int total, SyncManagerBase *syncer)
	{
		HandleWithHash (done, total, syncer, TCRows_,
				tr ("Audio transcoding"), tr ("Transcoding..."));
	}

	void ProgressManager::handleUploadProgress (int done, int total, SyncManagerBase *syncer)
	{
		HandleWithHash (done, total, syncer, UpRows_,
				tr ("Audio upload"), tr ("Uploading..."));
	}
}
}

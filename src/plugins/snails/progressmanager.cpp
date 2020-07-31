/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "progressmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <util/xpc/util.h>
#include "account.h"

namespace LC
{
namespace Snails
{
	ProgressManager::ProgressManager (QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	{
		Model_->setColumnCount (3);
	}

	QAbstractItemModel* ProgressManager::GetRepresentation () const
	{
		return Model_;
	}

	ProgressListener_ptr ProgressManager::MakeProgressListener (const QString& context)
	{
		const auto pl = std::make_shared<ProgressListener> ();
		const ProgressListener_wptr weakPl { pl };

		connect (pl.get (),
				&ProgressListener::started,
				this,
				[this, weakPl, context]
				{
					const QList<QStandardItem*> row
					{
						new QStandardItem { context },
						new QStandardItem { tr ("Running") },
						new QStandardItem { {} }
					};
					for (const auto item : row)
						item->setEditable (false);

					Util::InitJobHolderRow (row);

					Model_->appendRow (row);

					QMutexLocker locker { &Listener2RowMutex_ };
					if (!weakPl.expired ())
						Listener2Row_ [weakPl] = row;
				});

		connect (pl.get (),
				&ProgressListener::destroyed,
				this,
				[this, weakPl]
				{
					QList<QStandardItem*> row;
					{
						QMutexLocker locker { &Listener2RowMutex_ };
						row = Listener2Row_.take (weakPl);
					}

					if (!row.isEmpty ())
						Model_->removeRow (row.first ()->row ());
				});

		connect (pl.get (),
				&ProgressListener::gotProgress,
				this,
				[this, weakPl] (quint64 done, quint64 total)
				{
					QList<QStandardItem*> row;
					{
						QMutexLocker locker { &Listener2RowMutex_ };
						row = Listener2Row_ [weakPl];
					}

					if (!row.isEmpty ())
						Util::SetJobHolderProgress (row, done, total, "%1/%2");
				});

		return pl;
	}
}
}

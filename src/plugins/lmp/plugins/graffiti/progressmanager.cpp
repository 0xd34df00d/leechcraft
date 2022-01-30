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
#include "cuesplitter.h"

namespace LC::LMP::Graffiti
{
	ProgressManager::ProgressManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Model_->setColumnCount (3);
	}

	QAbstractItemModel* ProgressManager::GetModel () const
	{
		return Model_;
	}

	void ProgressManager::HandleTagsFetch (int fetched, int total, QObject *obj)
	{
		if (!TagsFetchObj2Row_.contains (obj))
		{
			const QList<QStandardItem*> row
			{
				new QStandardItem { tr ("Fetching tags...") },
				new QStandardItem { tr ("Fetching...") },
				new QStandardItem {}
			};
			auto item = row.at (JobHolderColumn::JobProgress);
			item->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
					CustomDataRoles::RoleJobHolderRow);

			TagsFetchObj2Row_ [obj] = row;
			Model_->appendRow (row);
		}

		if (fetched == total)
		{
			const auto& list = TagsFetchObj2Row_.take (obj);
			Model_->removeRow (list.first ()->row ());
			return;
		}

		const auto& list = TagsFetchObj2Row_ [obj];

		auto item = list.at (JobHolderColumn::JobProgress);
		item->setText (tr ("%1 of %2").arg (fetched).arg (total));
		Util::SetJobHolderProgress (item, fetched, total);
	}

	void ProgressManager::HandleCueSplitter (CueSplitter *splitter)
	{
		const QList<QStandardItem*> row
		{
			new QStandardItem (tr ("Splitting CUE %1...").arg (splitter->GetCueFile ())),
			new QStandardItem (tr ("Splitting...")),
			new QStandardItem ()
		};

		auto item = row.at (JobHolderColumn::JobProgress);
		item->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
				CustomDataRoles::RoleJobHolderRow);

		Splitter2Row_ [splitter] = row;
		Model_->appendRow (row);

		connect (splitter,
				&CueSplitter::splitProgress,
				this,
				[this, splitter] (int done, int total)
				{
					if (!Splitter2Row_.contains (splitter))
					{
						qWarning () << "unknown splitter";
						return;
					}

					Util::SetJobHolderProgress (Splitter2Row_ [splitter], done, total,
							tr ("%1 of %2").arg (done).arg (total));
				});
		connect (splitter,
				&CueSplitter::finished,
				this,
				[this, splitter]
				{
					if (Splitter2Row_.contains (splitter))
						Model_->removeRow (Splitter2Row_.take (splitter).constFirst ()->row ());
				});
	}
}

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

namespace LC
{
namespace LMP
{
namespace Graffiti
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

	void ProgressManager::handleTagsFetch (int fetched, int total, QObject *obj)
	{
		if (!TagsFetchObj2Row_.contains (obj))
		{
			auto nameItem = new QStandardItem (tr ("Fetching tags..."));
			auto statusItem = new QStandardItem (tr ("Fetching..."));
			auto progressItem = new QStandardItem ();

			const QList<QStandardItem*> row
			{
				nameItem,
				statusItem,
				progressItem
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

	void ProgressManager::handleCueSplitter (CueSplitter *splitter)
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
				SIGNAL (splitProgress (int, int, CueSplitter*)),
				this,
				SLOT (handleSplitProgress (int, int, CueSplitter*)));
		connect (splitter,
				SIGNAL (finished (CueSplitter*)),
				this,
				SLOT (handleSplitFinished (CueSplitter*)));
	}

	void ProgressManager::handleSplitProgress (int done, int total, CueSplitter *splitter)
	{
		if (!Splitter2Row_.contains (splitter))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown splitter";
			return;
		}

		if (done == total)
			return;

		Util::SetJobHolderProgress (Splitter2Row_ [splitter], done, total,
				tr ("%1 of %2").arg (done).arg (total));
	}

	void ProgressManager::handleSplitFinished (CueSplitter *splitter)
	{
		if (!Splitter2Row_.contains (splitter))
			return;

		Model_->removeRow (Splitter2Row_.take (splitter).first ()->row ());
	}
}
}
}

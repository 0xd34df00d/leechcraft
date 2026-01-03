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

namespace LC::LMP
{
	ProgressManager::ProgressManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
	}

	ProgressManager::Handle::Handle (QList<QStandardItem*> row, const QString& statusPattern, std::optional<int> total)
	: Row_ { std::move (row) }
	, StatusPattern_ { statusPattern }
	, Total_ { total.value_or (0) }
	{
	}

	ProgressManager::Handle::~Handle ()
	{
		if (!Row_.isEmpty ())
		{
			const auto item = Row_.value (0);
			item->model ()->removeRow (item->row ());
		}
	}

	void ProgressManager::Handle::Update (int done, int total)
	{
		Done_ = done;
		Total_ = total;
		Update ();
	}

	void ProgressManager::Handle::Update (int done)
	{
		Done_ = done;
		Update ();
	}

	void ProgressManager::Handle::operator++ ()
	{
		++Done_;
		Update ();
	}

	void ProgressManager::Handle::Update () const
	{
		Util::SetJobHolderProgress (Row_, Done_, Total_, StatusPattern_);
	}

	QAbstractItemModel* ProgressManager::GetModel () const
	{
		return Model_;
	}

	ProgressManager::Handle ProgressManager::Add (const Item& item)
	{
		const QList row { new QStandardItem { item.Name_ }, new QStandardItem {}, new QStandardItem {} };
		const auto progress = row.at (JobHolderColumn::JobProgress);
		progress->setData (QVariant::fromValue (item.Type_), CustomDataRoles::RoleJobHolderRow);
		Model_->appendRow (row);

		return Handle { row, item.StatusPattern_, item.Total_ };
	}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "exceptionsmodel.h"
#include <QSettings>

namespace LC
{
namespace CertMgr
{
	ExceptionsModel::ExceptionsModel (QSettings& settings, QObject *parent)
	: QStandardItemModel { parent }
	, Settings_ (settings)
	{
	}

	void ExceptionsModel::Populate ()
	{
		auto keys = Settings_.allKeys ();
		std::sort (keys.begin (), keys.end ());

		for (const auto& key : keys)
			Add (key, Settings_.value (key).toBool ());
	}

	void ExceptionsModel::ToggleState (const QModelIndex& index)
	{
		const auto& statusIdx = index.sibling (index.row (), Column::Status);

		const auto newVal = !statusIdx.data (IsAllowed).toBool ();
		const auto item = itemFromIndex (statusIdx);
		item->setText (newVal ?
					tr ("allow") :
					tr ("deny"));
		item->setData (newVal, IsAllowed);

		const auto& keyIndex = index.sibling (index.row (), Column::Name);
		Settings_.setValue (keyIndex.data ().toString (), newVal);
	}

	void ExceptionsModel::Add (const QString& key, bool val)
	{
		QList<QStandardItem*> row
		{
			new QStandardItem { key },
			new QStandardItem { val ? tr ("allow") : tr ("deny") }
		};

		for (auto item : row)
			item->setEditable (false);

		row.at (ExceptionsModel::Status)->setData (val, ExceptionsModel::IsAllowed);

		appendRow (row);
	}
}
}

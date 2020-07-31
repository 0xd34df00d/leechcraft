/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "headermodel.h"

using namespace LC::Plugins::NetworkMonitor;

LC::Plugins::NetworkMonitor::HeaderModel::HeaderModel (QObject *parent)
: QStandardItemModel { parent }
{
	setHorizontalHeaderLabels ({ tr ("Name"), tr ("Value") });
}

void HeaderModel::AddHeader (const QString& name, const QString& value)
{
	const QList<QStandardItem*> items
	{
		new QStandardItem { name },
		new QStandardItem { value }
	};
	for (const auto item : items)
		item->setEditable (false);
	appendRow (items);
}

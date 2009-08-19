/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "headermodel.h"

using namespace LeechCraft::Plugins::NetworkMonitor;

LeechCraft::Plugins::NetworkMonitor::HeaderModel::HeaderModel (QObject *parent)
: QStandardItemModel (parent)
{
	setHorizontalHeaderLabels (QStringList (tr ("Name"))
			<< tr ("Value"));
}

void HeaderModel::AddHeader (const QString& name, const QString& value)
{
	QList<QStandardItem*> items;
	items.push_back (new QStandardItem (name));
	items.push_back (new QStandardItem (value));
	appendRow (items);
}


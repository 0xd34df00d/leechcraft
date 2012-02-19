/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "progressmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "account.h"

namespace LeechCraft
{
namespace Snails
{
	ProgressManager::ProgressManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel)
	{
		Model_->setColumnCount (3);
	}

	QAbstractItemModel* ProgressManager::GetRepresentation () const
	{
		return Model_;
	}

	void ProgressManager::AddAccount (Account *acc)
	{
		connect (acc,
				SIGNAL (gotProgressListener (ProgressListener_g_ptr)),
				this,
				SLOT (handlePL (ProgressListener_g_ptr)));
	}

	void ProgressManager::handlePL (ProgressListener_g_ptr pl)
	{
		if (!pl)
			return;

		connect (pl,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handlePLDestroyed (QObject*)));
		connect (pl,
				SIGNAL (progress (int, int)),
				this,
				SLOT (handleProgress (int, int)));

		QList<QStandardItem*> row;
		row << new QStandardItem (pl->GetContext ());
		row << new QStandardItem (tr ("Running..."));
		row << new QStandardItem (QString (""));
		Model_->appendRow (row);

		Listener2Row_ [pl] = row.last ();
	}

	void ProgressManager::handlePLDestroyed (QObject *obj)
	{
		QStandardItem *item = Listener2Row_.take (obj);
		if (!item)
			return;

		Model_->removeRow (item->row ());
	}

	void ProgressManager::handleProgress (const int done, const int total)
	{
		auto item = Listener2Row_.value (sender ());
		if (!item)
			return;

		item->setText (QString ("%1/%2").arg (done).arg (total));
	}
}
}

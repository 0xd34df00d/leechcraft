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

#include "showconfigdialog.h"
#include <QStandardItemModel>

namespace LeechCraft
{
namespace Sidebar
{
	ShowConfigDialog::ShowConfigDialog (const QString& context, QWidget *parent)
	: QDialog (parent)
	, Context_ (context)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ActionsView_->setModel (Model_);
	}

	bool ShowConfigDialog::CheckAction (const QString& id, QAction *act)
	{
		for (int i = 0, size = Model_->rowCount (); i < size; ++i)
		{
			auto item = Model_->item (i);
			if (item->data (Roles::ActionID).toString () == id)
			{
				const bool result = item->checkState () == Qt::Checked;
				if (!result)
					HiddenActions_ [id] << act;
				return result;
			}
		}

		QStandardItem *item = new QStandardItem (act->icon (), act->text ());
		item->setToolTip (act->toolTip ());
		item->setCheckState (Qt::Checked);
		item->setData (id, Roles::ActionID);
		Model_->appendRow (item);

		return true;
	}
}
}

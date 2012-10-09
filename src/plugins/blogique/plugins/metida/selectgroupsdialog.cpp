/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "selectgroupsdialog.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "ljprofile.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	SelectGroupsDialog::SelectGroupsDialog (LJProfile *profile, quint32 allowMask,
			QWidget *parent)
	: QDialog (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.Groups_->setModel (Model_);
		Model_->setHorizontalHeaderLabels ({ tr ("Group") });

		for (const auto& group : profile->GetProfileData ().FriendGroups_)
		{
			QStandardItem *item = new QStandardItem (group.Name_);
			item->setData (group.Id_);
			item->setCheckable (true);
			if (allowMask & 1 << group.Id_)
				item->setCheckState (Qt::Checked);
			Model_->appendRow (item);
		}
	}

	QList<uint> SelectGroupsDialog::GetSelectedGroupsIds () const
	{
		QList<uint> result;
		for (int i = 0; i < Model_->rowCount (); ++i)
			if (Model_->item (i)->checkState () == Qt::Checked)
				result << Model_->item (i)->data ().toUInt ();

		return result;
	}

}
}
}


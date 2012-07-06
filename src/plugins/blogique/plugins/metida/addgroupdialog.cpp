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

#include "addgroupdialog.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	AddGroupDialog::AddGroupDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	QString AddGroupDialog::GetGroupName () const
	{
		return Ui_.GroupName_->text ();
	}

	void AddGroupDialog::SetGroupName (const QString& name)
	{
		Ui_.GroupName_->setText (name);
	}

	bool AddGroupDialog::GetAcccess () const
	{
		return Ui_.Public_->isChecked ();
	}

	void AddGroupDialog::SetAccess (bool isPublic)
	{
		Ui_.Public_->setChecked (isPublic);
	}

}
}
}


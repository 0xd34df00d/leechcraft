/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "userscriptsmanager.h"
#include <QDebug>
#include <QStandardItemModel> 

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{

	UserScriptsManager::UserScriptsManager (QStandardItemModel* model)
	: Model_ (model)
	{
		Ui_.setupUi (this);
		Ui_.Items_->setModel (model);
	}

	void UserScriptsManager::on_Edit__released ()
	{
		QModelIndex selected = Ui_.Items_->currentIndex ();
	}

	void UserScriptsManager::on_Disable__released ()
	{
		QModelIndex selected = Ui_.Items_->currentIndex ();

		if (selected.isValid ())
		{
			
		}

	}

	void UserScriptsManager::on_Remove__released ()
	{
		QModelIndex selected = Ui_.Items_->currentIndex ();

		if (selected.isValid ())
		{
			Ui_.Items_->model ()->removeRow (selected.row ());
		}
	}
}
}
}


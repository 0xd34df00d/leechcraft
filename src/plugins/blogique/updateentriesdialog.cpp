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

#include "updateentriesdialog.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
	UpdateEntriesDialog::UpdateEntriesDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Ui_.EntriesCount_->setValue (XmlSettingsManager::Instance ()
				.Property ("LastLocalEntriesToView", 20).toInt ());
	}

	int UpdateEntriesDialog::GetCount () const
	{
		return  Ui_.EntriesCount_->value ();
	}

	void UpdateEntriesDialog::accept ()
	{
		XmlSettingsManager::Instance ()
				.setProperty ("LocalLoadAsk", !Ui_.UpdateAsk_->isChecked ());
		XmlSettingsManager::Instance ()
				.setProperty ("LastLocalEntriesToView", Ui_.EntriesCount_->value ());
		QDialog::accept ();
	}

}
}

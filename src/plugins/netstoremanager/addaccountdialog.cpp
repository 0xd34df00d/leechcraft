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

#include "addaccountdialog.h"
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	AddAccountDialog::AddAccountDialog (const QList<IStoragePlugin*>& plugins, QWidget *w)
	: QDialog (w)
	{
		Ui_.setupUi (this);

		Q_FOREACH (auto plugin, plugins)
		{
			const QString& name = plugin->GetStorageName ();
			const QIcon& icon = plugin->GetStorageIcon ();
			Ui_.Storage_->addItem (icon, name,
					QVariant::fromValue<IStoragePlugin*> (plugin));
		}
	}

	QString AddAccountDialog::GetAccountName () const
	{
		return Ui_.AccountName_->text ();
	}

	IStoragePlugin* AddAccountDialog::GetStoragePlugin () const
	{
		const int idx = Ui_.Storage_->currentIndex ();
		if (idx < 0)
			return 0;

		return Ui_.Storage_->itemData (idx).value<IStoragePlugin*> ();
	}
}
}

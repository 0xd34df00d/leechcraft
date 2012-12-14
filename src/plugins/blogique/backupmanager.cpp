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

#include "backupmanager.h"
#include "interfaces/blogique/iaccount.h"
#include "core.h"
#include "accountsselectdialog.h"

namespace LeechCraft
{
namespace Blogique
{
	BackupManager::BackupManager (QObject *parent)
	: QObject (parent)
	{
	}

	void BackupManager::backup ()
	{
		const auto& accounts = Core::Instance ().GetAccounts ();
		AccountsSelectDialog dlg;
		dlg.FillAccounts (accounts);
		if (dlg.exec () == QDialog::Rejected)
			return;

		for (auto acc : dlg.GetSelectedAccounts ())
			acc->backup ();
	}

	void BackupManager::backup (IAccount*)
	{
	}

}
}

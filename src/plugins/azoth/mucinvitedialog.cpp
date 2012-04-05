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

#include "mucinvitedialog.h"
#include <QtDebug>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	MUCInviteDialog::MUCInviteDialog (IAccount *acc, QWidget *parent)
	: QDialog (parent)
	, ManualMode_ (false)
	{
		Ui_.setupUi (this);
		Ui_.Invitee_->setInsertPolicy (QComboBox::NoInsert);

		Q_FOREACH (QObject *entryObj, acc->GetCLEntries ())
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry ||
					entry->GetEntryType () != ICLEntry::ETChat)
				continue;

			const QString& id = entry->GetHumanReadableID ();
			Ui_.Invitee_->addItem (QString ("%1 (%2)")
						.arg (entry->GetEntryName ())
						.arg (id),
					id);
		}
	}

	QString MUCInviteDialog::GetID () const
	{
		const int idx = Ui_.Invitee_->currentIndex ();
		return (idx >= 0 && !ManualMode_) ?
				Ui_.Invitee_->itemData (idx).toString () :
				Ui_.Invitee_->currentText ();
	}

	QString MUCInviteDialog::GetMessage () const
	{
		return Ui_.Message_->text ();
	}

	void MUCInviteDialog::on_Invitee__currentIndexChanged ()
	{
		ManualMode_ = false;
	}

	void MUCInviteDialog::on_Invitee__editTextChanged ()
	{
		ManualMode_ = true;
	}
}
}

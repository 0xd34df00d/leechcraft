/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mucinvitedialog.h"
#include <QtDebug>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"

namespace LC
{
namespace Azoth
{
	MUCInviteDialog::MUCInviteDialog (IAccount *acc, ListType type, QWidget *parent)
	: QDialog (parent)
	, ManualMode_ (false)
	{
		Ui_.setupUi (this);
		Ui_.Invitee_->setInsertPolicy (QComboBox::NoInsert);

		ICLEntry::EntryType requestedType = ICLEntry::EntryType::Chat;
		switch (type)
		{
		case ListType::ListEntries:
			break;
		case ListType::ListMucs:
			requestedType = ICLEntry::EntryType::MUC;
			Ui_.InviteeLabel_->setText ("Conferences:");
			break;
		}

		for (auto entryObj : acc->GetCLEntries ())
		{
			const auto entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry ||
					entry->GetEntryType () != requestedType)
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

	void MUCInviteDialog::SetID (const QString& id)
	{
		for (int i = 0; i < Ui_.Invitee_->count (); ++i)
			if (Ui_.Invitee_->itemData (i).toString () == id)
			{
				Ui_.Invitee_->setCurrentIndex (i);
				ManualMode_ = false;
				return;
			}

		Ui_.Invitee_->setEditText (id);

		ManualMode_ = true;
	}

	QString MUCInviteDialog::GetInviteMessage () const
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

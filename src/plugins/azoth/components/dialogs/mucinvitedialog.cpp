/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mucinvitedialog.h"
#include <variant>
#include <QLineEdit>
#include <QPushButton>
#include <QtDebug>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/trackingpointer.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucentry.h"
#include "util/azoth/util.h"

namespace LC::Azoth
{
	namespace
	{
		bool IsMuc (const ICLEntry& entry)
		{
			return entry.GetEntryType () == ICLEntry::EntryType::MUC;
		}

		bool MightInvite (const ICLEntry& muc, const ICLEntry& invitee)
		{
			return qobject_ref_cast<const IMUCEntry> (muc.GetQObject ()).CanInvite (invitee) == IMUCEntry::MightInvite;
		}
	}

	MUCInviteDialogBase::MUCInviteDialogBase (const ICLEntry& source, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.Invitee_->setInsertPolicy (QComboBox::NoInsert);

		const bool sourceIsMuc = IsMuc (source);
		if (!sourceIsMuc)
			Ui_.InviteeLabel_->setText (tr ("Conferences:"));

		const auto isOffered = [&] (const ICLEntry& entry)
		{
			return sourceIsMuc ?
					!IsMuc (entry) && MightInvite (source, entry) :
					IsMuc (entry) && MightInvite (entry, source);
		};

		for (const auto entry : source.GetParentAccount ()->GetCLEntries ())
			if (isOffered (*entry))
				Ui_.Invitee_->addItem (FormatNameWithAddress (*entry), QVariant::fromValue (Util::TrackingPointer { entry }));

		if (!Ui_.Invitee_->count ())
			Ui_.Invitee_->setPlaceholderText (sourceIsMuc ?
					tr ("Nobody can be invited to this conference") :
					tr ("This contact cannot be invited to any conference"));
	}

	QString MUCInviteDialogBase::GetInviteMessage () const
	{
		return Ui_.Message_->text ();
	}

	template<AddressInvitations AI>
	MUCInviteDialog<AI>::MUCInviteDialog (const ICLEntry& source, QWidget *parent)
	: MUCInviteDialogBase { source, parent }
	{
		Ui_.Invitee_->setEditable (AI == AddressInvitations::Allowed);
		if constexpr (AI == AddressInvitations::Allowed)
			Ui_.Invitee_->lineEdit ()->setPlaceholderText (tr ("Contact or address to invite"));
		else if (!Ui_.Invitee_->count ())
			Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (false);
	}

	template<AddressInvitations AI>
	MUCInviteDialog<AI>::TargetEntry_t MUCInviteDialog<AI>::GetTargetEntry () const
	{
		const auto idx = Ui_.Invitee_->currentIndex ();
		const auto entry = Ui_.Invitee_->itemData (idx).value<Util::TrackingPointer<ICLEntry>> ().GetOrNull ();

		if constexpr (AI == AddressInvitations::Allowed)
		{
			if (const auto& text = Ui_.Invitee_->currentText ().trimmed ();
				text != Ui_.Invitee_->itemText (idx).trimmed ())
				return text;
		}

		return entry;
	}

	template class MUCInviteDialog<AddressInvitations::Disallowed>;
	template class MUCInviteDialog<AddressInvitations::Allowed>;
}

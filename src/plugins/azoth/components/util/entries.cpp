/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entries.h"
#include <ranges>
#include <variant>
#include <QInputDialog>
#include <QSettings>
#include <QMessageBox>
#include <util/sll/prelude.h>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/visitor.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include "util/azoth/util.h"

#ifdef ENABLE_CRYPT
#include <interfaces/azoth/isupportpgp.h>
#include "components/dialogs/pgpkeyselectiondialog.h"
#endif

#include "../../core.h"

namespace LC::Azoth
{
	void AuthorizeEntry (ICLEntry *entry)
	{
		const auto account = entry->GetParentAccount ();
		account->Authorize (entry->GetQObject ());
		account->RequestAuth (entry->GetConventionalId ().ToString ());

		const auto& e = Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryId ());
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void DenyAuthForEntry (ICLEntry *entry)
	{
		const auto account = entry->GetParentAccount ();
		account->DenyAuth (entry->GetQObject ());

		const auto& e = Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryId ());
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	bool ChoosePGPKey (ISupportPGP *pgp, ICLEntry *entry)
	{
#ifdef ENABLE_CRYPT
		const auto persistentId = entry->GetGlobalPersistentId ();
		if (!persistentId)
		{
			QMessageBox::critical (nullptr,
					QObject::tr ("LeechCraft"),
					QObject::tr ("PGP keys can only be bound to contacts with a persistent identifier. "
								"Unfortunately, %1 does not have one on the protocol level.")
						.arg (FormatNameWithAddress (*entry)));
			qWarning () << "no persistent ID for" << entry->GetHumanReadableAddress ();
			return false;
		}

		const auto& str = QObject::tr ("Please select the key for %1.")
				.arg (FormatNameWithAddress (*entry));
		PGPKeySelectionDialog dia { str, PGPKeySelectionDialog::TPublic,
				pgp->GetEntryKey (entry->GetQObject ()) };
		if (dia.exec () != QDialog::Accepted)
			return false;

		const auto& key = dia.GetSelectedKey ();

		pgp->SetEntryKey (entry->GetQObject (), key);

		QSettings settings { QCoreApplication::organizationName (), QCoreApplication::applicationName () + "_Azoth" };
		settings.beginGroup ("PublicEntryKeys");
		if (key.isNull ())
			settings.remove (persistentId->ToString ());
		else
			settings.setValue (persistentId->ToString (), key.keyId ());
		settings.endGroup ();

		return !key.isNull ();
#else
		return false;
#endif
	}

	QStringList GetMucParticipants (const EntriesManager& entries, const GlobalPersistentId& entryId)
	{
		auto& entry = entries.GetEntry<IMUCEntry> (entryId);
		return Util::Map (entry.GetParticipants (), &ICLEntry::GetEntryName);
	}

	namespace
	{
		struct Trs
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Azoth)
		};
	}

	InviteCheck_t CheckInvitation (ICLEntry& entry, ICLEntry& target)
	{
		const auto entryMuc = qobject_cast<IMUCEntry*> (entry.GetQObject ());
		const auto targetMuc = qobject_cast<IMUCEntry*> (target.GetQObject ());
		if (static_cast<bool> (entryMuc) == static_cast<bool> (targetMuc))
			return NotAnInvitation {};

		const auto entryAcc = entry.GetParentAccount ();
		const auto targetAcc = target.GetParentAccount ();
		if (entryAcc->GetParentProtocol () != targetAcc->GetParentProtocol ())
			return NotAnInvitation {};

		const auto muc = entryMuc ? entryMuc : targetMuc;
		auto& invitee = entryMuc ? target : entry;

		const auto features = muc->GetMUCFeatures ();
		if (!features.testFlag (IMUCEntry::MUCFCanInvite))
			return InviteRefusal { FormatInviteRefusal (invitee, *muc, IMUCEntry::CannotInvite::UnsupportedFeature) };

		if (entryAcc != targetAcc)
		{
			if (features.testFlag (IMUCEntry::MUCFCanInviteByAddress))
				return InvitePlan { muc, invitee.GetHumanReadableAddress (), FormatNameWithAddress (invitee) };

			return InviteRefusal { Trs::tr ("Cannot invite %1 to %2 because they belong to different accounts and the conference can't invite by address.")
					.arg (FormatNameWithAddress (invitee), FormatNameWithAddress (muc->AsCLEntry ())) };
		}

		const auto check = muc->CanInvite (invitee);
		if (const auto reason = std::get_if<IMUCEntry::CannotInvite> (&check))
			return InviteRefusal { FormatInviteRefusal (invitee, *muc, *reason) };

		return InvitePlan { muc, &invitee, FormatNameWithAddress (invitee) };
	}

	void TryInvite (const QList<Util::TrackingPointer<ICLEntry>>& entries,
			const Util::TrackingPointer<ICLEntry>& target,
			QWidget *parent)
	{
		for (const auto& entry : entries)
		{
			if (!entry || !target)
				continue;

			Util::Visit (CheckInvitation (*entry, *target),
					[] (NotAnInvitation) {},
					[&] (const InviteRefusal& refusal)
					{
						QMessageBox::warning (parent, Trs::tr ("Conference invitation"), refusal.Message_);
					},
					[&] (const InvitePlan& plan)
					{
						bool accepted = false;
						const auto& inviteMessage = QInputDialog::getText (parent,
								Trs::tr ("Conference invitation"),
								Trs::tr ("Do you want to invite %1 to %2? You can add an optional invitation message:")
										.arg (plan.InviteeLabel_, FormatNameWithAddress (plan.Muc_->AsCLEntry ())),
								QLineEdit::Normal,
								{},
								&accepted);
						if (!accepted || !target || !entry)
							return;

						Util::Visit (plan.Invitee_,
								[&] (ICLEntry *invitee)
								{
									plan.Muc_->InviteToMUC (*invitee, inviteMessage);
								},
								[&] (const QString& address)
								{
									plan.Muc_->InviteToMUC (address, inviteMessage);
								});
					});
		}
	}
}

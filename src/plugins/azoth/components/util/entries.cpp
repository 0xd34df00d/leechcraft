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
#include <QSettings>
#include <util/gui/asyncdialogs.h>
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

	namespace
	{
		Util::Task<std::optional<QString>> GetInviteMessage (QWidget *parent, const QString& label)
		{
			return Util::Dialogs::GetText ({
						.Parent_ = parent,
						.Title_ = Trs::tr ("Conference invitation"),
						.Label_ = label + '\n' + Trs::tr ("You can add an optional invitation message:"),
					});
		}

		QString MakeInviteMessageLabel (const QList<InvitePlan>& plans)
		{
			const auto& first = plans [0];
			if (plans.size () == 1)
				return Trs::tr ("Do you want to invite %1 to %2?")
						.arg (first.InviteeLabel_, FormatNameWithAddress (first.Muc_->AsCLEntry ()));

			// by construction, all invites are either of the same entry to multiple rooms,
			// or of multiple entries to the same room
			enum class MultiInviteType : bool
			{
				SameEntryToManyRooms,
				ManyEntriesToSameRoom
			};

			const auto inviteType = plans [0].Muc_ == plans [1].Muc_ ?
					MultiInviteType::ManyEntriesToSameRoom :
					MultiInviteType::SameEntryToManyRooms;
			switch (inviteType)
			{
			case MultiInviteType::SameEntryToManyRooms:
				return Trs::tr ("Do you want to invite %1 to %n rooms?", nullptr, plans.size ())
						.arg (first.InviteeLabel_);
			case MultiInviteType::ManyEntriesToSameRoom:
				return Trs::tr ("Do you want to invite %n entries to %1?", nullptr, plans.size ())
						.arg (FormatNameWithAddress (first.Muc_->AsCLEntry ()));
			}
			std::unreachable ();
		}
	}

	Util::Task<void> TryInvite (QList<Util::TrackingPointer<ICLEntry>> entries,
			Util::TrackingPointer<ICLEntry> target,
			QPointer<QWidget> parent)
	{
		QList<InviteRefusal> refusals;
		QList<InvitePlan> plans;

		for (const auto& entry : entries)
		{
			if (!entry || !target)
				continue;

			Util::Visit (CheckInvitation (*entry, *target),
					[] (NotAnInvitation) {},
					[&] (const InviteRefusal& refusal) { refusals << refusal; },
					[&] (const InvitePlan& plan) { plans << plan; });
		}

		const auto warnGuard = Util::MakeScopeGuard ([&]
				{
					if (!refusals.isEmpty ())
						Util::Dialogs::Warning (parent,
								Trs::tr ("Conference invitation"),
								Util::Map (refusals, &InviteRefusal::Message_).join ('\n'));
				});

		if (plans.isEmpty ())
			co_return;

		if (const auto& msg = co_await GetInviteMessage (parent, MakeInviteMessageLabel (plans)))
			for (const auto& plan : plans)
			{
				if (!plan.Muc_)
					continue;

				Util::Visit (plan.Invitee_,
						[&] (const Util::TrackingPointer<ICLEntry>& entry)
						{
							if (entry)
								plan.Muc_->InviteToMUC (*entry, *msg);
						},
						[&] (const QString& address) { plan.Muc_->InviteToMUC (address, *msg); });
			}
	}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entries.h"
#include <ranges>
#include <QSettings>
#include <QMessageBox>
#include <util/sll/prelude.h>
#include <util/sll/qobjectrefcast.h>
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
		account->RequestAuth (entry->GetConventionalID ().ToString ());

		const auto& e = Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryID ());
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void DenyAuthForEntry (ICLEntry *entry)
	{
		const auto account = entry->GetParentAccount ();
		account->DenyAuth (entry->GetQObject ());

		const auto& e = Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryID ());
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	ICLEntry* FindByHRId (IAccount *acc, const QString& hrId)
	{
		const auto& allEntries = acc->GetCLEntries ();
		const auto pos = std::ranges::find_if (allEntries,
				[&hrId] (ICLEntry *entry)
				{
					return entry->GetConventionalID ().ToString () == hrId;
				});
		return pos == allEntries.end () ? nullptr : *pos;
	}

	bool ChoosePGPKey (ISupportPGP *pgp, ICLEntry *entry)
	{
#ifdef ENABLE_CRYPT
		const auto& address = entry->GetHumanReadableAddress ();

		const auto persistentId = entry->GetGlobalPersistentID ();
		if (!persistentId)
		{
			QMessageBox::critical (nullptr,
					QObject::tr ("LeechCraft"),
					QObject::tr ("PGP keys can only be bound to contacts with a persistent identifier. "
								"Unfortunately, %1 (%2) does not have one on the protocol level.")
						.arg (entry->GetEntryName (), address));
			qWarning () << "no persistent ID for" << address;
			return false;
		}

		const auto& str = QObject::tr ("Please select the key for %1 (%2).")
				.arg (entry->GetEntryName ())
				.arg (address);
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

	QStringList GetMucParticipants (const GlobalPersistentId& entryId)
	{
		auto& entry = Core::Instance ().GetEntry<IMUCEntry> (entryId);
		return Util::Map (entry.GetParticipants (), &ICLEntry::GetEntryName);
	}
}

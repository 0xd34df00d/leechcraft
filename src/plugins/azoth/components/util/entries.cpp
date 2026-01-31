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
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>

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
		const auto& id = entry->GetHumanReadableID ();
		account->Authorize (entry->GetQObject ());
		account->RequestAuth (id);

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

	QObject* FindByHRId (IAccount *acc, const QString& hrId)
	{
		const auto& allEntries = acc->GetCLEntries ();
		const auto pos = std::ranges::find_if (allEntries,
				[&hrId] (QObject *obj)
				{
					return qobject_cast<ICLEntry*> (obj)->GetHumanReadableID () == hrId;
				});
		return pos == allEntries.end () ? nullptr : *pos;
	}

	bool ChoosePGPKey (ISupportPGP *pgp, ICLEntry *entry)
	{
#ifdef ENABLE_CRYPT
		const auto& str = QObject::tr ("Please select the key for %1 (%2).")
				.arg (entry->GetEntryName ())
				.arg (entry->GetHumanReadableID ());
		PGPKeySelectionDialog dia { str, PGPKeySelectionDialog::TPublic,
				pgp->GetEntryKey (entry->GetQObject ()) };
		if (dia.exec () != QDialog::Accepted)
			return false;

		const auto& key = dia.GetSelectedKey ();

		pgp->SetEntryKey (entry->GetQObject (), key);

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth" };
		settings.beginGroup ("PublicEntryKeys");
		if (key.isNull ())
			settings.remove (entry->GetEntryID ());
		else
			settings.setValue (entry->GetEntryID (), key.keyId ());
		settings.endGroup ();

		return !key.isNull ();
#else
		return false;
#endif
	}

	QStringList GetMucParticipants (const QString& entryId)
	{
		const auto entry = qobject_cast<IMUCEntry*> (Core::Instance ().GetEntry (entryId));
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry
					<< "doesn't implement IMUCEntry";
			return {};
		}

		QStringList participantsList;
		for (const auto item : entry->GetParticipants ())
		{
			const auto part = qobject_cast<ICLEntry*> (item);
			if (!part)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast item to ICLEntry"
						<< item;
				continue;
			}
			participantsList << part->GetEntryName ();
		}
		return participantsList;
	}

}

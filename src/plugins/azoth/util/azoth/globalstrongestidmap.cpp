/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "globalstrongestidmap.h"
#include <util/sll/visitor.h>
#include "interfaces/azoth/iclentry.h"

namespace LC::Azoth
{
	void GlobalStrongestIdMap::AddEntry (ICLEntry& entry)
	{
		const auto& id = entry.GetGlobalStrongestID ();
		Entries_ [id] = &entry;
		RegisterAccount (entry.GetParentAccount ());
	}

	ICLEntry* GlobalStrongestIdMap::GetEntry (const GlobalStrongestId& id) const
	{
		return Entries_.value (id);
	}

	void GlobalStrongestIdMap::RegisterAccount (IAccount *acc)
	{
		if (KnownAccounts_.contains (acc))
			return;

		const auto accId = acc->GetAccountID ();
		KnownAccounts_ << acc;
		connect (acc->GetQObject (),
				&QObject::destroyed,
				this,
				[this, acc, accId]
				{
					KnownAccounts_.remove (acc);
					Entries_.removeIf ([accId] (const std::pair<GlobalStrongestId, ICLEntry*>& pair)
							{
								return GetAccountId (pair.first) == accId;
							});
				});

		auto& emitter = acc->GetAccountEmitter ();
		connect (&emitter,
				&Emitters::Account::conventionalIdChanged,
				this,
				[this] (const GlobalConventionalId& oldId, const GlobalConventionalId& newId)
				{
					if (const auto entry = Entries_.take (oldId))
						Entries_ [newId] = entry;
				});
		connect (&emitter,
				&Emitters::Account::removedCLItems,
				this,
				[this] (const QList<ICLEntry*>& entries)
				{
					for (const auto entry : entries)
						Entries_.remove (entry->GetGlobalStrongestID ());
				});
	}
}

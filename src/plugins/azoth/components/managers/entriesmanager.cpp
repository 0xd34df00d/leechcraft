/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entriesmanager.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucentry.h"
#include "util/azoth/util.h"

namespace LC::Azoth
{
	void EntriesManager::AddAccount (IAccount& account)
	{
		AddCLEntries (account.GetCLEntries ());

		auto& emitter = account.GetAccountEmitter ();
		connect (&emitter,
				&Emitters::Account::gotCLItems,
				this,
				&EntriesManager::AddCLEntries);
		connect (&emitter,
				&Emitters::Account::removedCLItems,
				this,
				&EntriesManager::RemoveCLEntries);
		connect (&emitter,
				&Emitters::Account::conventionalIdChanged,
				this,
				[this] (const GlobalConventionalId& oldId, const GlobalConventionalId& newId, const ICLEntry& entry)
				{
					const auto taken = ID2Entry_.take (oldId);
					if (taken != &entry)
						qCritical () << "updating" << oldId << "->" << newId << "— the entry wasn't under oldId";
					if (std::exchange (ID2Entry_ [newId], taken))
						qCritical () << "updating" << oldId << "->" << newId << "— another entry already is under newId";
					emit conventionalIdChanged (oldId, newId, entry);
				});
	}

	template<typename T>
	T& EntriesManager::GetEntry (const GlobalStrongestId& id) const
	{
		const auto entry = GetEntryOrNull<T> (id);
		if (!entry)
			throw std::runtime_error { "missing or invalid entry for ID " + id.ToString ().toStdString () };
		return *entry;
	}

	template<typename T>
	T* EntriesManager::GetEntryOrNull (const GlobalStrongestId& id) const
	{
		const auto entry = ID2Entry_.value (id);
		if (!entry)
			return nullptr;

		if constexpr (std::is_same_v<T, ICLEntry>)
			return entry;
		else if constexpr (std::is_same_v<T, QObject>)
			return entry->GetQObject ();
		else
			return qobject_cast<T*> (entry->GetQObject ());
	}

	void EntriesManager::AddCLEntries (const QList<ICLEntry*>& entries)
	{
		for (const auto entry : entries)
		{
			auto& existing = ID2Entry_ [entry->GetGlobalStrongestID ()];
			if (existing && existing != entry)
				qCritical () << "duplicate entry:" << entry->GetGlobalStrongestID ();
			existing = entry;
		}
	}

	void EntriesManager::RemoveCLEntries (const QList<ICLEntry*>& entries)
	{
		for (const auto entry : entries)
			if (!ID2Entry_.remove (entry->GetGlobalStrongestID ()))
				qCritical () << "removing non-existing entry:" << entry->GetGlobalStrongestID ();
	}

	template QObject& EntriesManager::GetEntry<QObject> (const GlobalStrongestId&) const;
	template ICLEntry& EntriesManager::GetEntry<ICLEntry> (const GlobalStrongestId&) const;
	template IMUCEntry& EntriesManager::GetEntry<IMUCEntry> (const GlobalStrongestId&) const;

	template QObject* EntriesManager::GetEntryOrNull<QObject> (const GlobalStrongestId&) const;
	template ICLEntry* EntriesManager::GetEntryOrNull<ICLEntry> (const GlobalStrongestId&) const;
	template IMUCEntry* EntriesManager::GetEntryOrNull<IMUCEntry> (const GlobalStrongestId&) const;
}

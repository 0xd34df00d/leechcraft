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
#include "interfaces/azoth/ihaveavatars.h"
#include "interfaces/azoth/imucentry.h"
#include "util/azoth/util.h"
#include "accountsmanager.h"

namespace LC::Azoth
{
	EntriesManager::EntriesManager (const AccountsManager& accounts, QObject *parent)
	: QObject { parent }
	{
		accounts.OnEachAccount (*this, std::bind_front (&EntriesManager::AddAccount, this));
	}

	void EntriesManager::AddAccount (IAccount& account)
	{
		AddCLEntries (account.GetCLEntries (), account);

		auto& emitter = account.GetAccountEmitter ();
		connect (&emitter,
				&Emitters::Account::gotCLItems,
				this,
				[this, &account] (const auto& entries) { AddCLEntries (entries, account); });
		connect (&emitter,
				&Emitters::Account::removedCLItems,
				this,
				[this, &account] (const auto& entries) { RemoveCLEntries (entries, account); });
		connect (&emitter,
				&Emitters::Account::conventionalIdChanged,
				this,
				[this] (const GlobalConventionalId& oldId, const GlobalConventionalId& newId, const ICLEntry& entry)
				{
					if (entry.GetGlobalStrongestId () != newId)
						return;

					if (const auto pos = Id2Entry_.find (oldId);
						pos != Id2Entry_.end () && *pos == &entry)
					{
						const auto taken = *pos;
						Id2Entry_.erase (pos);
						if (std::exchange (Id2Entry_ [newId], taken))
							qCritical () << "updating" << oldId << "->" << newId << "— another entry already is under newId";
					}
					else
						qCritical () << "updating" << oldId << "->" << newId << "— the entry wasn't under oldId";

					if (const auto pos = SessionStableIds_.find (oldId);
						pos != SessionStableIds_.end ())
					{
						const auto taken = *pos;
						SessionStableIds_.erase (pos);
						if (const auto prevId = std::exchange (SessionStableIds_ [newId], taken);
							prevId != StableSentinel)
						{
							emit stableIdShadowed (prevId);
							emit Emitter_.stableIdShadowed (prevId);
						}
					}

					emit strongestIdChanged (oldId, newId, entry);
					emit Emitter_.strongestIdChanged (oldId, newId, entry);
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
		const auto entry = Id2Entry_.value (id);
		if (!entry)
			return nullptr;

		if constexpr (std::is_same_v<T, ICLEntry>)
			return entry;
		else if constexpr (std::is_same_v<T, QObject>)
			return entry->GetQObject ();
		else
			return qobject_cast<T*> (entry->GetQObject ());
	}

	std::optional<quint64> EntriesManager::GetSessionStableId (const GlobalStrongestId& id) const
	{
		const auto pos = SessionStableIds_.find (id);
		return pos == SessionStableIds_.end () ? std::nullopt : std::optional { *pos };
	}

	void EntriesManager::AddCLEntries (const QList<ICLEntry*>& entries, IAccount& account)
	{
		QList<ICLEntry*> addedEntries;
		addedEntries.reserve (entries.size ());

		for (const auto entry : entries)
		{
			const auto id = entry->GetGlobalStrongestId ();
			auto& existing = Id2Entry_ [id];
			if (existing && existing != entry)
				qCritical () << "duplicate entry:" << id;

			if (!existing)
			{
				addedEntries << entry;
				existing = entry;

				if (!SessionStableIds_.contains (id))
					SessionStableIds_ [id] = StableCounter_++;
			}
		}

		if (!addedEntries.isEmpty ())
		{
			emit added (addedEntries, account);
			emit Emitter_.added (addedEntries, account);
		}
	}

	void EntriesManager::RemoveCLEntries (const QList<ICLEntry*>& entries, IAccount& account)
	{
		if (entries.isEmpty ())
			return;

		emit Emitter_.aboutToBeRemoved (entries, account);
		emit aboutToBeRemoved (entries, account);

		for (const auto entry : entries)
			if (!Id2Entry_.remove (entry->GetGlobalStrongestId ()))
				qCritical () << "removing non-existing entry:" << entry->GetGlobalStrongestId ();

		emit removed (entries, account);
		emit Emitter_.removed (entries, account);
	}

	template QObject& EntriesManager::GetEntry<QObject> (const GlobalStrongestId&) const;
	template ICLEntry& EntriesManager::GetEntry<ICLEntry> (const GlobalStrongestId&) const;
	template IMUCEntry& EntriesManager::GetEntry<IMUCEntry> (const GlobalStrongestId&) const;
	template IHaveAvatars& EntriesManager::GetEntry<IHaveAvatars> (const GlobalStrongestId&) const;

	template QObject* EntriesManager::GetEntryOrNull<QObject> (const GlobalStrongestId&) const;
	template ICLEntry* EntriesManager::GetEntryOrNull<ICLEntry> (const GlobalStrongestId&) const;
	template IMUCEntry* EntriesManager::GetEntryOrNull<IMUCEntry> (const GlobalStrongestId&) const;
	template IHaveAvatars* EntriesManager::GetEntryOrNull<IHaveAvatars> (const GlobalStrongestId&) const;
}

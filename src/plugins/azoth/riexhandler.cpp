/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "riexhandler.h"
#include <QtDebug>
#include <util/sll/unreachable.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "components/dialogs/acceptriexdialog.h"

namespace LC
{
namespace Azoth
{
namespace RIEX
{
	namespace
	{
		void FilterRIEXItems (QList<RIEXItem>& items, const QHash<QString, ICLEntry*>& clEntries)
		{
			const auto end = std::remove_if (items.begin (), items.end (),
					[&clEntries] (const RIEXItem& item)
					{
						const auto entry = clEntries.value (item.ID_);

						switch (item.Action_)
						{
						case RIEXItem::AAdd:
							return entry != nullptr;
						case RIEXItem::ADelete:
							if (item.Groups_.isEmpty ())
								return false;
							else
							{
								const auto& origGroups = entry->Groups ();
								return std::any_of (item.Groups_.begin (), item.Groups_.end (),
										[&origGroups] (const auto& group) { return origGroups.contains (group); });
							}
						case RIEXItem::AModify:
							return !entry;
						}

						Util::Unreachable ();
					});

			items.erase (end, items.end ());
		}

		void AddRIEX (const RIEXItem& item, const QHash<QString, ICLEntry*> entries, IAccount *acc)
		{
			if (!entries.contains (item.ID_))
			{
				acc->RequestAuth (item.ID_, QString (), item.Nick_, item.Groups_);
				return;
			}

			ICLEntry *entry = entries [item.ID_];

			const auto allGroups = std::all_of (item.Groups_.begin (), item.Groups_.end (),
					[entry] (const QString& group) { return entry->Groups ().contains (group); });

			if (!allGroups)
			{
				QStringList newGroups = item.Groups_ + entry->Groups ();
				newGroups.removeDuplicates ();
				entry->SetGroups (newGroups);
			}
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping already-existing"
						<< item.ID_;
				return;
			}
		}

		void ModifyRIEX (const RIEXItem& item, const QHash<QString, ICLEntry*> entries, IAccount*)
		{
			if (!entries.contains (item.ID_))
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping non-existent"
						<< item.ID_;
				return;
			}

			ICLEntry *entry = entries [item.ID_];

			if (!item.Groups_.isEmpty ())
				entry->SetGroups (item.Groups_);

			if (!item.Nick_.isEmpty ())
				entry->SetEntryName (item.Nick_);
		}

		void DeleteRIEX (const RIEXItem& item, const QHash<QString, ICLEntry*> entries, IAccount *acc)
		{
			if (!entries.contains (item.ID_))
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping non-existent"
						<< item.ID_;
				return;
			}

			const auto entry = entries [item.ID_];
			if (item.Groups_.isEmpty ())
				acc->RemoveEntry (entry->GetQObject ());
			else
			{
				auto newGroups = entry->Groups ();
				for (const auto& group : item.Groups_)
					newGroups.removeAll (group);

				entry->SetGroups (newGroups);
			}
		}
	}

	void HandleRIEXItemsSuggested (QList<RIEXItem> items, QObject *from, QString message)
	{
		if (items.isEmpty () || !from)
			return;

		const auto entry = qobject_cast<ICLEntry*> (from);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< from
					<< "doesn't implement ICLEntry";
			return;
		}

		const auto acc = entry->GetParentAccount ();
		QHash<QString, ICLEntry*> clEntries;
		for (const auto entryObj : acc->GetCLEntries ())
		{
			const auto entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry ||
					(entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) != ICLEntry::FPermanentEntry)
				continue;

			clEntries [entry->GetHumanReadableID ()] = entry;
		}

		FilterRIEXItems (items, clEntries);
		if (items.isEmpty ())
			return;

		AcceptRIEXDialog dia (items, from, message);
		if (dia.exec () != QDialog::Accepted)
			return;

		for (const auto& item : dia.GetSelectedItems ())
		{
			switch (item.Action_)
			{
			case RIEXItem::AAdd:
				AddRIEX (item, clEntries, acc);
				break;
			case RIEXItem::AModify:
				ModifyRIEX (item, clEntries, acc);
				break;
			case RIEXItem::ADelete:
				DeleteRIEX (item, clEntries, acc);
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown action"
						<< item.Action_
						<< "for item"
						<< item.ID_
						<< item.Nick_
						<< item.Groups_;
				break;
			}
		}
	}
}
}
}

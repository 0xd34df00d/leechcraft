/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "riexhandler.h"
#include <QtDebug>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "acceptriexdialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace RIEX
{
	namespace
	{
		void FilterRIEXItems (QList<RIEXItem>& items, const QHash<QString, ICLEntry*>& clEntries)
		{
			Q_FOREACH (const RIEXItem& item, items)
			{
				ICLEntry *entry = clEntries.value (item.ID_);
				if (!entry &&
						(item.Action_ == RIEXItem::AModify ||
						item.Action_ == RIEXItem::ADelete))
				{
					qWarning () << Q_FUNC_INFO
							<< "skipping non-existent"
							<< item.ID_;
					items.removeAll (item);
					continue;
				}

				if (item.Action_ == RIEXItem::ADelete &&
						entry &&
						!item.Groups_.isEmpty ())
				{
					bool found = false;
					const QStringList& origGroups = entry->Groups ();
					Q_FOREACH (const QString& group, item.Groups_)
						if (origGroups.contains (group))
						{
							found = true;
							break;
						}

					if (!found)
						items.removeAll (item);
				}

				if (item.Action_ == RIEXItem::AAdd &&
						entry)
					items.removeAll (item);
			}
		}

		void AddRIEX (const RIEXItem& item, const QHash<QString, ICLEntry*> entries, IAccount *acc)
		{
			if (!entries.contains (item.ID_))
			{
				acc->RequestAuth (item.ID_, QString (), item.Nick_, item.Groups_);
				return;
			}

			ICLEntry *entry = entries [item.ID_];

			bool allGroups = true;
			Q_FOREACH (const QString& group, item.Groups_)
				if (!entry->Groups ().contains (group))
				{
					allGroups = false;
					break;
				}

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

		void ModifyRIEX (const RIEXItem& item, const QHash<QString, ICLEntry*> entries, IAccount *acc)
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

			ICLEntry *entry = entries [item.ID_];
			if (item.Groups_.isEmpty ())
				acc->RemoveEntry (entry->GetObject ());
			else
			{
				QStringList newGroups = entry->Groups ();
				Q_FOREACH (const QString& group, item.Groups_)
					newGroups.removeAll (group);

				entry->SetGroups (newGroups);
			}
		}
	}

	void HandleRIEXItemsSuggested (QList<RIEXItem> items, QObject *from, QString message)
	{
		if (items.isEmpty () || !from)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (from);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< from
					<< "doesn't implement ICLEntry";
			return;
		}

		IAccount *acc = qobject_cast<IAccount*> (entry->GetParentAccount ());
		QHash<QString, ICLEntry*> clEntries;
		Q_FOREACH (QObject *entryObj, acc->GetCLEntries ())
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
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

		Q_FOREACH (const RIEXItem& item, dia.GetSelectedItems ())
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

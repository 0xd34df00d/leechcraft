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

#include "groupmanager.h"
#include <QtDebug>
#include "msnaccount.h"
#include "msnbuddyentry.h"
#include "callbacks.h"
#include "zheetutil.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	GroupManager::GroupManager (Callbacks *cb, MSNAccount *acc)
	: QObject (acc)
	, Account_ (acc)
	, CB_ (cb)
	{
		connect (CB_,
				SIGNAL (gotGroups (QList<MSN::Group>)),
				this,
				SLOT (handleGotGroups (QList<MSN::Group>)));
		connect (CB_,
				SIGNAL(removedGroup (QString)),
				this,
				SLOT (handleRemovedGroup (QString)));
		connect (CB_,
				SIGNAL (renamedGroup (QString, QString)),
				this,
				SLOT (handleRenamedGroup (QString, QString)));

		connect (CB_,
				SIGNAL (buddyAddedToGroup (QString, QString)),
				this,
				SLOT (handleBuddyAdded (QString, QString)));
		connect (CB_,
				SIGNAL (buddyRemovedFromGroup (QString, QString)),
				this,
				SLOT (handleBuddyRemoved (QString, QString)));
	}

	void GroupManager::SetGroups (MSNBuddyEntry *entry,
			const QStringList& newGroupsLst, const QStringList& oldGroupsLst)
	{
		const auto& newSet = newGroupsLst.toSet ();
		const auto& oldSet = oldGroupsLst.toSet ();

		Q_FOREACH (const QString& grp, newSet - oldSet)
			AddGroup (entry->GetHumanReadableID (), grp);

		Q_FOREACH (const QString& grp, oldSet - newSet)
			RemoveGroup (entry->GetHumanReadableID (), grp);
	}

	void GroupManager::AddGroup (const QString& entry, const QString& name)
	{
		qDebug () << Q_FUNC_INFO << entry << name << Group2ID_;
		auto conn = Account_->GetNSConnection ();

		if (Group2ID_.contains (name))
		{
			const auto& id = Group2ID_ [name];
			const auto& cid = Account_->GetBuddy (entry)->GetContactID ();
			conn->addToGroup (ZheetUtil::ToStd (id), ZheetUtil::ToStd (cid));
		}
		else
		{
			conn->addGroup (ZheetUtil::ToStd (name));
			PendingAdditions_ [name] << entry;
		}
	}

	void GroupManager::RemoveGroup (const QString& entry, const QString& name)
	{
		qDebug () << Q_FUNC_INFO << entry << name << Group2ID_;
		if (!Group2ID_.contains (name))
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown group"
					<< name;
			return;
		}

		const auto& id = ZheetUtil::ToStd (Group2ID_ [name]);
		const auto& entryId = ZheetUtil::ToStd (Account_->GetBuddy (entry)->GetContactID ());
		Account_->GetNSConnection ()->removeFromGroup (id, entryId);
	}

	void GroupManager::handleGotGroups (const QList<MSN::Group>& groups)
	{
		Q_FOREACH (const MSN::Group& g, groups)
		{
			const auto& name = ZheetUtil::FromStd (g.name);
			const auto& id = ZheetUtil::FromStd (g.groupID);
			Group2ID_ [name] = id;
			ID2Group_ [id] = name;

			Q_FOREACH (const QString& id, PendingAdditions_.take (name))
				AddGroup (id, name);
		}
	}

	void GroupManager::handleRemovedGroup (const QString& id)
	{
		Group2ID_.remove (ID2Group_.take (id));
	}

	void GroupManager::handleRenamedGroup (const QString& id, const QString& name)
	{
		Group2ID_.remove (ID2Group_.take (id));

		Group2ID_ [name] = id;
		ID2Group_ [id] = name;
	}

	void GroupManager::handleBuddyAdded (const QString& id, const QString& groupId)
	{
		Account_->GetBuddyByCID (id)->AddGroup (ID2Group_ [groupId]);
	}

	void GroupManager::handleBuddyRemoved (const QString& id, const QString& groupId)
	{
		Account_->GetBuddyByCID (id)->RemoveGroup (ID2Group_ [groupId]);
	}
}
}
}

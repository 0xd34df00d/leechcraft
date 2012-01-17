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
#include "proto/connection.h"
#include "mrimaccount.h"
#include "mrimbuddy.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	GroupManager::GroupManager (MRIMAccount *parent)
	: QObject (parent)
	, A_ (parent)
	, Conn_ (parent->GetConnection ())
	{
		connect (Conn_,
				SIGNAL (gotGroups (QStringList)),
				this,
				SLOT (handleGotGroups (QStringList)));
		connect (Conn_,
				SIGNAL (contactAdded (quint32, quint32)),
				this,
				SLOT (handleGroupAdded (quint32, quint32)));
	}

	QString GroupManager::GetGroup (int num) const
	{
		return ID2Group_.value (num);
	}

	int GroupManager::GetGroupNumber (const QString& group) const
	{
		return Group2ID_.value (group, 0);
	}

	void GroupManager::SetBuddyGroups (MRIMBuddy *buddy, const QStringList& groups)
	{
		if (groups.isEmpty () || Group2ID_.contains (groups.at (0)))
		{
			const quint32 gid = groups.isEmpty () ?
					0 :
					Group2ID_ [groups.at (0)];
			Conn_->ModifyContact (buddy->GetID (), gid,
					buddy->GetHumanReadableID (), buddy->GetEntryName ());
			buddy->SetGroup (groups.value (0));
			return;
		}

		const QString& group = groups.at (0);
		if (!PendingContacts_.contains (group))
		{
			const quint32 seq = Conn_->AddGroup (groups.at (0), Group2ID_.size ());
			PendingGroups_ [seq] = group;
		}

		PendingContacts_ [group] << buddy;
	}

	void GroupManager::handleGotGroups (const QStringList& list)
	{
		int i = 0;
		Q_FOREACH (const QString& g, list)
		{
			ID2Group_ [i] = g;
			Group2ID_ [g] = i;
			++i;
		}
	}

	void GroupManager::handleGroupAdded (quint32 seq, quint32 groupId)
	{
		if (!PendingGroups_.contains (seq))
			return;

		const QString& group = PendingGroups_.take (seq);
		Group2ID_ [group] = groupId;
		ID2Group_ [groupId] = group;

		Q_FOREACH (MRIMBuddy *buddy, PendingContacts_.take (group))
			SetBuddyGroups (buddy, QStringList (group));
	}
}
}
}

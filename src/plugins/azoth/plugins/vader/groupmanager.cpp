/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupmanager.h"
#include "proto/connection.h"
#include "mrimaccount.h"
#include "mrimbuddy.h"

namespace LC
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
					buddy->GetHumanReadableID (), buddy->GetEntryName (), buddy->GetInfo ().Phone_);
			buddy->SetGroup (groups.value (0));
			return;
		}

		const auto& group = groups.at (0);
		if (!PendingContacts_.contains (group))
		{
			const auto seq = Conn_->AddGroup (groups.at (0), Group2ID_.size ());
			PendingGroups_ [seq] = group;
		}

		PendingContacts_ [group] << buddy;
	}

	void GroupManager::handleGotGroups (const QStringList& list)
	{
		int i = 0;
		for (const auto& g : list)
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

		const auto& group = PendingGroups_.take (seq);
		Group2ID_ [group] = groupId;
		ID2Group_ [groupId] = group;

		for (const auto buddy : PendingContacts_.take (group))
			SetBuddyGroups (buddy, { group });
	}
}
}
}

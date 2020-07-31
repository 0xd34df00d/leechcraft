/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupsmanager.h"
#include <QTimer>
#include <QStringList>
#include "vkconnection.h"
#include "structures.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	GroupsManager::GroupsManager (VkConnection *conn)
	: QObject (conn)
	, Conn_ (conn)
	{
		connect (Conn_,
				SIGNAL (gotLists (QList<ListInfo>)),
				this,
				SLOT (handleLists (QList<ListInfo>)));
		connect (Conn_,
				SIGNAL (addedLists (QList<ListInfo>)),
				this,
				SLOT (handleAddedLists (QList<ListInfo>)));
		connect (Conn_,
				SIGNAL (gotUsers (QList<UserInfo>)),
				this,
				SLOT (handleUsers (QList<UserInfo>)));
	}

	ListInfo GroupsManager::GetListInfo (qulonglong id) const
	{
		return ID2ListInfo_ [id];
	}

	ListInfo GroupsManager::GetListInfo (const QString& name) const
	{
		const auto pos = std::find_if (ID2ListInfo_.begin (), ID2ListInfo_.end (),
				[&name] (const ListInfo& li) { return li.Name_ == name; });
		return pos == ID2ListInfo_.end () ? ListInfo () : *pos;
	}

	void GroupsManager::UpdateGroups (const QStringList& oldGroups,
			const QStringList& newGroups, qulonglong id)
	{
		for (const auto& newItem : newGroups)
		{
			if (oldGroups.contains (newItem))
				continue;

			const auto listPos = std::find_if (ID2ListInfo_.begin (), ID2ListInfo_.end (),
					[&newItem] (const ListInfo& li) { return li.Name_ == newItem; });
			if (listPos != ID2ListInfo_.end ())
			{
				const auto list = listPos->ID_;
				List2IDs_ [list] << id;

				ModifiedLists_ << list;
			}
			else
				NewLists_ [newItem] << id;
		}

		for (const auto& oldItem : oldGroups)
			if (!newGroups.contains (oldItem))
			{
				auto list = GetListInfo (oldItem).ID_;
				List2IDs_ [list].remove (id);

				ModifiedLists_ << list;
			}

		if (!IsApplyScheduled_)
		{
			IsApplyScheduled_ = true;
			QTimer::singleShot (1000,
					this,
					SLOT (applyChanges ()));
		}
	}

	void GroupsManager::applyChanges ()
	{
		for (auto list : ModifiedLists_)
			Conn_->ModifyFriendList (GetListInfo (list), List2IDs_ [list].values ());

		for (auto i = NewLists_.begin (), end = NewLists_.end (); i != end; ++i)
			Conn_->AddFriendList (i.key (), i.value ().values ());

		ModifiedLists_.clear ();

		IsApplyScheduled_ = false;
	}

	void GroupsManager::handleLists (const QList<ListInfo>& lists)
	{
		ID2ListInfo_.clear ();
		for (const auto& list : lists)
			ID2ListInfo_ [list.ID_] = list;
	}

	void GroupsManager::handleAddedLists (const QList<ListInfo>& lists)
	{
		for (const auto& list : lists)
		{
			ID2ListInfo_ [list.ID_] = list;

			List2IDs_ [list.ID_] += NewLists_.take (list.Name_);
		}
	}

	void GroupsManager::handleUsers (const QList<UserInfo>& infos)
	{
		for (const auto& info : infos)
			for (const auto& listId : info.Lists_)
				List2IDs_ [listId] << info.ID_;
	}
}
}
}

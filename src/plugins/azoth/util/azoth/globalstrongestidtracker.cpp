/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "globalstrongestidtracker.h"
#include <util/sll/visitor.h>
#include "interfaces/azoth/iclentry.h"

namespace LC::Azoth
{
	GlobalStrongestIdTracker::GlobalStrongestIdTracker (ICLEntry& entry, QObject *parent)
	: QObject { parent }
	, Id_ { entry.GetGlobalStrongestID ()}
	{
		Util::Visit (Id_,
				[] (const GlobalPersistentId&) {},
				[this, &entry] (const GlobalConventionalId&)
				{
					connect (&entry.GetParentAccount ()->GetAccountEmitter (),
							&Emitters::Account::conventionalIdChanged,
							this,
							[this] (const GlobalConventionalId& oldId, const GlobalConventionalId& newId)
							{
								if (oldId == Id_)
									Id_ = newId;
							});
				});
	}

	GlobalStrongestId GlobalStrongestIdTracker::GetId () const
	{
		return Id_;
	}

	GlobalStrongestIdTracker::operator GlobalStrongestId () const
	{
		return Id_;
	}
}

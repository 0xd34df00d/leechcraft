/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QtDebug>
#include <interfaces/azoth/iclentry.h>

namespace LC
{
namespace Azoth
{
namespace VelvetBird
{
	State FromPurpleState (PurpleStatusPrimitive state)
	{
		switch (state)
		{
		case PURPLE_STATUS_OFFLINE:
			return State::SOffline;
		case PURPLE_STATUS_AVAILABLE:
			return State::SOnline;
		case PURPLE_STATUS_UNAVAILABLE:
			return State::SDND;
		case PURPLE_STATUS_INVISIBLE:
			return State::SInvisible;
		case PURPLE_STATUS_AWAY:
			return State::SAway;
		case PURPLE_STATUS_EXTENDED_AWAY:
			return State::SXA;
		default:
			return State::SInvalid;
		}
	}

	PurpleStatusPrimitive ToPurpleState (State state)
	{
		switch (state)
		{
		case State::SOffline:
			return PURPLE_STATUS_OFFLINE;
		case State::SOnline:
		case State::SChat:
			return PURPLE_STATUS_AVAILABLE;
		case State::SAway:
			return PURPLE_STATUS_AWAY;
		case State::SXA:
			return PURPLE_STATUS_EXTENDED_AWAY;
		case State::SDND:
			return PURPLE_STATUS_UNAVAILABLE;
		default:
			return PURPLE_STATUS_UNSET;
		}
	}

	EntryStatus FromPurpleStatus (PurpleAccount *account, PurpleStatus *status)
	{
		const auto id = purple_status_get_id (status);
		const auto statusType = purple_account_get_status_type (account, id);

		const auto message = purple_status_get_attr_string (status, "message");

		return EntryStatus (FromPurpleState (purple_status_type_get_primitive (statusType)),
				message ? QString::fromUtf8 (message) : QString ());
	}
}
}
}

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

#include "util.h"
#include <interfaces/azoth/iclentry.h>

namespace LeechCraft
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

	EntryStatus FromPurpleStatus (PurpleStatus *status)
	{
		const auto id = purple_status_get_id (status);
		const auto message = purple_status_get_attr_string (status, "message");
		return EntryStatus (FromPurpleState (purple_primitive_get_type_from_id (id)),
				message ? QString::fromUtf8 (message) : QString ());
	}
}
}
}

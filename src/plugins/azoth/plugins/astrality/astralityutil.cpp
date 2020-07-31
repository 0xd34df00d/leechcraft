/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "astralityutil.h"
#include <Presence>
#include <interfaces/azoth/iclentry.h>

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	State StateTelepathy2Azoth (Tp::ConnectionPresenceType type)
	{
		switch (type)
		{
		case Tp::ConnectionPresenceTypeOffline:
			return SOffline;
		case Tp::ConnectionPresenceTypeAvailable:
			return SOnline;
		case Tp::ConnectionPresenceTypeAway:
			return SAway;
		case Tp::ConnectionPresenceTypeBusy:
			return SDND;
		case Tp::ConnectionPresenceTypeExtendedAway:
			return SXA;
		case Tp::ConnectionPresenceTypeHidden:
			return SInvisible;
		case Tp::ConnectionPresenceTypeUnset:
		case Tp::ConnectionPresenceTypeUnknown:
		case Tp::ConnectionPresenceTypeError:
			return SInvalid;
		default:
			return SInvalid;
		}
	}

	Tp::ConnectionPresenceType StateAzoth2Telepathy (State state)
	{
		switch (state)
		{
		case SOffline:
			return Tp::ConnectionPresenceTypeOffline;
		case SOnline:
		case SChat:
			return Tp::ConnectionPresenceTypeAvailable;
		case SAway:
			return Tp::ConnectionPresenceTypeAway;
		case SDND:
			return Tp::ConnectionPresenceTypeBusy;
		case SXA:
			return Tp::ConnectionPresenceTypeExtendedAway;
		case SInvisible:
			return Tp::ConnectionPresenceTypeHidden;
		case SInvalid:
		case SProbe:
		case SConnecting:
			return Tp::ConnectionPresenceTypeUnknown;
		case SError:
			return Tp::ConnectionPresenceTypeError;
		default:
			return Tp::ConnectionPresenceTypeUnknown;
		}
	}

	Tp::Presence Status2Telepathy (const EntryStatus& status)
	{
		switch (status.State_)
		{
			case SOffline:
				return Tp::Presence::offline (status.StatusString_);
			case SOnline:
			case SChat:
				return Tp::Presence::available (status.StatusString_);
			case SAway:
				return Tp::Presence::away (status.StatusString_);
			case SDND:
				return Tp::Presence::busy (status.StatusString_);
			case SXA:
				return Tp::Presence::xa (status.StatusString_);
			case SInvisible:
				return Tp::Presence::hidden (status.StatusString_);
			default:
				return Tp::Presence ();
		}
	}

	EntryStatus Status2Azoth (const Tp::Presence& p)
	{
		return EntryStatus (StateTelepathy2Azoth (p.type ()), p.statusMessage ());
	}
}
}
}

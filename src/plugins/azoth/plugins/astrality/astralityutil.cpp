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

#include "astralityutil.h"

namespace LeechCraft
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
			return Tp::ConnectionPresenceTypeUnknown;
		default:
			return Tp::ConnectionPresenceTypeUnknown;
		}
	}
}
}
}

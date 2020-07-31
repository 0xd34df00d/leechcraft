/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <Constants>
#include <interfaces/azoth/azothcommon.h>

namespace Tp
{
	class Presence;
}

namespace LC
{
namespace Azoth
{
struct EntryStatus;

namespace Astrality
{
	State StateTelepathy2Azoth (Tp::ConnectionPresenceType);
	Tp::ConnectionPresenceType StateAzoth2Telepathy (State);
	Tp::Presence Status2Telepathy (const EntryStatus&);
	EntryStatus Status2Azoth (const Tp::Presence&);
}
}
}

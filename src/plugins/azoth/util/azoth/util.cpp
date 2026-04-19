/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"

namespace LC::Azoth
{
	bool IsOnline (State st)
	{
		switch (st)
		{
		case SOffline:
		case SError:
		case SConnecting:
		case SInvalid:
			return false;
		default:
			return true;
		}
	}

	QString StateToString (State st)
	{
		switch (st)
		{
		case SOnline:
			return QObject::tr ("Online");
		case SChat:
			return QObject::tr ("Free to chat");
		case SAway:
			return QObject::tr ("Away");
		case SDND:
			return QObject::tr ("Do not disturb");
		case SXA:
			return QObject::tr ("Not available");
		case SOffline:
			return QObject::tr ("Offline");
		default:
			return QObject::tr ("Error");
		}
	}
}

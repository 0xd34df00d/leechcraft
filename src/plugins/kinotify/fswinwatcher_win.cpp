/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012 Dimitriy Ryazantcev
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fswinwatcher.h"
#include <windows.h>
#include <basetyps.h>
#include <shellapi.h>

namespace LC
{
namespace Kinotify
{
	FSWinWatcher::FSWinWatcher (ICoreProxy_ptr, QObject *parent)
	: QObject (parent)
	{
	}

	bool FSWinWatcher::IsCurrentFS ()
	{
		QUERY_USER_NOTIFICATION_STATE state;
		if (SHQueryUserNotificationState (&state) != S_OK)
			return false;
		return state != QUNS_ACCEPTS_NOTIFICATIONS;
	}
}
}

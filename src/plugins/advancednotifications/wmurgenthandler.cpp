/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "wmurgenthandler.h"
#include <QMainWindow>
#include <QApplication>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include "generalhandler.h"

namespace LC::AdvancedNotifications
{
	NotificationMethod WMUrgentHandler::GetHandlerMethod () const
	{
		return NMUrgentHint;
	}

	void WMUrgentHandler::Handle (const Entity& e, const NotificationRule&)
	{
		if (e.Additional_ [AN::EF::EventCategory].toString () == AN::CatEventCancel)
			return;

		bool ok = false;
		auto winIdx = e.Additional_ [AN::EF::WindowIndex].toInt (&ok);

		auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();

		if (winIdx < 0 || winIdx >= rootWM->GetWindowsCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid window index"
					<< winIdx
					<< "for notification"
					<< e.Additional_;
			winIdx = rootWM->GetPreferredWindowIndex ();
		}

		auto win = rootWM->GetMainWindow (ok ? winIdx : rootWM->GetPreferredWindowIndex ());

		if (!win->isActiveWindow ())
			QApplication::alert (win);
	}
}

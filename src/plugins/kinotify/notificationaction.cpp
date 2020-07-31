/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationaction.h"

namespace LC
{
namespace Kinotify
{
	NotificationAction::NotificationAction (QObject *parent)
	: QObject (parent)
	, ActionObject_ (0)
	{
	}

	void NotificationAction::SetActionObject (QObject* obj)
	{
		ActionObject_ = obj;
	}

	void NotificationAction::sendActionOnClick (const QString& idx)
	{
		QMetaObject::invokeMethod (ActionObject_,
				"notificationActionTriggered",
				Qt::QueuedConnection,
				Q_ARG (int, idx.toInt ()));

		emit actionPressed ();
	}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationactionhandler.h"
#include <QStringList>

namespace LC::Util
{
	NotificationActionHandler::NotificationActionHandler (Entity& e, QObject*)
	: Entity_ (e)
	{
		Entity_.Additional_ ["HandlingObject"] = QVariant::fromValue<QObject_ptr> (QObject_ptr (this));
	}

	void NotificationActionHandler::AddFunction (const QString& name, Callback_t callback)
	{
		ActionName2Callback_ << qMakePair (name, callback);
		const QStringList& sl = Entity_.Additional_ ["NotificationActions"].toStringList ();
		Entity_.Additional_ ["NotificationActions"] = sl + QStringList (name);
	}

	void NotificationActionHandler::AddDependentObject (QObject *obj)
	{
		DependentObjects_ << QPointer<QObject> (obj);
	}

	void NotificationActionHandler::notificationActionTriggered (int idx)
	{
		if (std::any_of (DependentObjects_.begin (), DependentObjects_.end (),
				[] (const auto& obj) { return obj.isNull (); }))
			return;

		ActionName2Callback_.at (idx).second ();
	}
}

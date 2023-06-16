/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationsmanager.h"
#include <QTimer>
#include <util/xpc/util.h>
#include <interfaces/an/constants.h>
#include <interfaces/core/ientitymanager.h>
#include "todostorage.h"

namespace LC
{
namespace Otlozhu
{
	NotificationsManager::NotificationsManager (TodoStorage *storage)
	: QObject (storage)
	, Storage_ (storage)
	, NextEventTimer_ (new QTimer (this))
	{
		NextEventTimer_->setSingleShot (true);
		connect (NextEventTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleTimer ()));

		connect (Storage_,
				SIGNAL (itemAdded (int)),
				this,
				SLOT (readjustTimer ()));
		connect (Storage_,
				SIGNAL (itemUpdated (int)),
				this,
				SLOT (readjustTimer ()));
		connect (Storage_,
				SIGNAL (itemRemoved (int)),
				this,
				SLOT (readjustTimer ()));
		readjustTimer ();
	}

	void NotificationsManager::handleTimer ()
	{
		const QString& notify = tr ("%1 is due now!").arg (NextEvent_->GetTitle ());
		auto e = Util::MakeNotification ("Otlozhu", notify, Priority::Info);

		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Otlozhu";
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = AN::CatOrganizer;
		e.Additional_ ["org.LC.AdvNotifications.EventID"] = "org.LC.Plugins.Otlozhu.EventDue/" + NextEvent_->GetID ();
		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (NextEvent_->GetTitle ());

		e.Additional_ ["org.LC.AdvNotifications.EventType"] = AN::TypeOrganizerEventDue;
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;

		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);

		QTimer::singleShot (1100,
				this,
				SLOT (readjustTimer ()));
	}

	void NotificationsManager::readjustTimer ()
	{
		NextEvent_.reset ();
		NextEventTimer_->stop ();

		QDateTime min;
		const auto& now = QDateTime::currentDateTime ();

		for (int i = 0, size = Storage_->GetNumItems (); i < size; ++i)
		{
			auto item = Storage_->GetItemAt (i);
			if (item->GetPercentage () == 100)
				continue;

			if (!item->GetDueDate ().isValid ())
				continue;

			const auto& due = item->GetDueDate ();
			if ((!min.isValid () || due < min) && due > now)
			{
				min = due;
				NextEvent_ = item;
			}
		}

		if (!min.isValid ())
			return;

		int secsDiff = now.secsTo (min);
		if (secsDiff > std::numeric_limits<int>::max () / 10000)
			QTimer::singleShot (std::numeric_limits<int>::max () / 10,
					this,
					SLOT (readjustTimer ()));
		else
			NextEventTimer_->start (secsDiff * 1000);
	}
}
}

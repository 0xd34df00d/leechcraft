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

#include "notificationsmanager.h"
#include <QTimer>
#include <util/util.h>
#include "todostorage.h"

namespace LeechCraft
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
		auto e = Util::MakeNotification ("Otlozhu", notify, PInfo_);

		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Otlozhu";
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = "org.LC.AdvNotifications.Organizer";
		e.Additional_ ["org.LC.AdvNotifications.EventID"] = "org.LC.Plugins.Otlozhu.EventDue/" + NextEvent_->GetID ();
		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (NextEvent_->GetTitle ());

		e.Additional_ ["org.LC.AdvNotifications.EventType"] = "org.LC.AdvNotifications.Organizer.EventDue";
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = notify;
		e.Additional_ ["org.LC.AdvNotifications.Count"] = 1;

		emit gotEntity (e);

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
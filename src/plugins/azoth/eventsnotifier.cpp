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

#include "eventsnotifier.h"
#include <util/util.h>
#include <util/notificationactionhandler.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "chattabsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	EventsNotifier::EventsNotifier (QObject *parent)
	: QObject (parent)
	{
	}

	void EventsNotifier::RegisterEntry (ICLEntry *entry)
	{
		QObject *entryObj = entry->GetObject ();

		connect (entryObj,
				SIGNAL (chatPartStateChanged (const ChatPartState&, const QString&)),
				this,
				SLOT (handleChatPartStateChanged (const ChatPartState&, const QString&)));
	}

	void EventsNotifier::handleChatPartStateChanged (const ChatPartState& state,
			const QString&)
	{
		if (state != CPSComposing)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLentry";
			return;
		}

		const QString& id = entry->GetEntryID ();
		if (!ShouldNotifyNext_.value (id, true))
			return;

		const QString& type = XmlSettingsManager::Instance ()
				.property ("NotifyIncomingComposing").toString ();
		if (type == "all" ||
			(type == "opened" &&
				Core::Instance ().GetChatTabsManager ()->IsOpenedChat (id)))
		{
			ShouldNotifyNext_ [id] = false;

			Entity e = Util::MakeNotification ("Azoth",
					tr ("%1 started composing a message to you.")
						.arg (entry->GetEntryName ()),
					PInfo_);
			e.Additional_ ["NotificationPixmap"] =
						QVariant::fromValue<QPixmap> (QPixmap::fromImage (entry->GetAvatar ()));
			Util::NotificationActionHandler *nh =
					new Util::NotificationActionHandler (e, this);
			nh->AddFunction (tr ("Open chat"),
					[entry] () { Core::Instance ().GetChatTabsManager ()->OpenChat (entry); });
			nh->AddDependentObject (entry->GetObject ());
			emit gotEntity (e);
		}
	}

	void EventsNotifier::handleEntryMadeCurrent (QObject *entryObj)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry";
			return;
		}

		ShouldNotifyNext_ [entry->GetEntryID ()] = true;
	}
}
}

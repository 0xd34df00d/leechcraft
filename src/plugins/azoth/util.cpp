/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "util.h"
#include <interfaces/structures.h>
#include "interfaces/iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	void BuildNotification (Entity& e, ICLEntry *other)
	{
		e.Additional_ ["NotificationPixmap"] =
				QVariant::fromValue<QPixmap> (QPixmap::fromImage (other->GetAvatar ()));
		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Azoth";
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] =
				"org.LC.AdvNotifications.IM";
		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth.IncomingMessageFrom/" + other->GetEntryID ();

		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (other->GetEntryName ());

		e.Additional_ ["org.LC.Plugins.Azoth.SourceName"] = other->GetEntryName ();
		e.Additional_ ["org.LC.Plugins.Azoth.SourceID"] = other->GetEntryID ();
	}
}
}

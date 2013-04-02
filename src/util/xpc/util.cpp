/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <util/util.h>

namespace LeechCraft
{
namespace Util
{
	Entity MakeAN (const QString& header, const QString& text, Priority priority,
			const QString& senderID, const QString& cat, const QString& type,
			const QString& id, const QStringList& visualPath,
			int delta, int count,
			const QString& fullText, const QString& extendedText)
	{
		auto e = MakeNotification (header, text, priority);
		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = senderID;
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = cat;
		e.Additional_ ["org.LC.AdvNotifications.EventID"] = id;
		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = visualPath;
		e.Additional_ ["org.LC.AdvNotifications.EventType"] = type;
		e.Additional_ ["org.LC.AdvNotifications.FullText"] = fullText.isNull () ? text : fullText;
		e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = extendedText.isNull () ? text : extendedText;
		if (delta)
			e.Additional_ ["org.LC.AdvNotifications.DeltaCount"] = delta;
		else
			e.Additional_ ["org.LC.AdvNotifications.Count"] = count;
		return e;
	}
}
}

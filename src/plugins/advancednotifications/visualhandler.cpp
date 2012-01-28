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

#include "visualhandler.h"
#include "core.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	VisualHandler::VisualHandler ()
	{
	}

	NotificationMethod VisualHandler::GetHandlerMethod () const
	{
		return NMVisual;
	}

	void VisualHandler::Handle (const Entity& orig, const NotificationRule&)
	{
		if (orig.Additional_ ["org.LC.AdvNotifications.EventCategory"].toString () == "org.LC.AdvNotifications.Cancel")
			return;

		const QString& evId = orig.Additional_ ["org.LC.AdvNotifications.EventID"].toString ();
		if (ActiveEvents_.contains (evId))
			return;

		Entity e = orig;

		if (e.Additional_ ["Text"].toString ().isEmpty ())
		{
			if (!e.Additional_ ["org.LC.AdvNotifications.FullText"].toString ().isEmpty ())
				e.Additional_ ["Text"] = e.Additional_ ["org.LC.AdvNotifications.FullText"];
			else if (!e.Additional_ ["org.LC.AdvNotifications.ExtendedText"].toString ().isEmpty ())
				e.Additional_ ["Text"] = e.Additional_ ["org.LC.AdvNotifications.ExtendedText"];
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "refusing to rehandle entity with empty text:"
						<< e.Entity_
						<< e.Additional_;
				return;
			}
		}

		Q_FOREACH (const QString& key, e.Additional_.keys ())
			if (key.startsWith ("org.LC.AdvNotifications."))
				e.Additional_.remove (key);

		if (e.Mime_.endsWith ("+advanced"))
			e.Mime_.remove ("+advanced");

		QObject_ptr probeObj (new QObject ());
		ActiveEvents_ << evId;
		probeObj->setProperty ("EventID", evId);
		connect (probeObj.get (),
				SIGNAL (destroyed ()),
				this,
				SLOT (handleProbeDestroyed ()));
		QVariant probe = QVariant::fromValue<QObject_ptr> (probeObj);
		e.Additional_ ["RemovalProbe"] = probe;

		Core::Instance ().SendEntity (e);
	}

	void VisualHandler::handleProbeDestroyed ()
	{
		const QString& evId = sender ()->property ("EventID").toString ();
		ActiveEvents_.remove (evId);
	}
}
}

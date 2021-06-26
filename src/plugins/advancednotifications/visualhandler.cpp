/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "visualhandler.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include "generalhandler.h"

namespace LC
{
namespace AdvancedNotifications
{
	NotificationMethod VisualHandler::GetHandlerMethod () const
	{
		return NMVisual;
	}

	void VisualHandler::Handle (const Entity& orig, const NotificationRule&)
	{
		if (orig.Additional_ [AN::EF::EventCategory].toString () == AN::CatEventCancel)
			return;

		const QString& evId = orig.Additional_ [AN::EF::EventID].toString ();
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

		for (auto i = e.Additional_.begin (); i !=  e.Additional_.end (); )
		{
			const auto& key = i.key ();
			if (key.startsWith ("org.LC.AdvNotifications."))
				i = e.Additional_.erase (i);
			else
				++i;
		}

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

		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void VisualHandler::handleProbeDestroyed ()
	{
		const QString& evId = sender ()->property ("EventID").toString ();
		ActiveEvents_.remove (evId);
	}
}
}

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
#include <interfaces/entityconstants.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include <util/sll/qtutil.h>

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

		if (e.Additional_ [EF::Text].toString ().isEmpty ())
		{
			if (!e.Additional_ [AN::EF::FullText].toString ().isEmpty ())
				e.Additional_ [EF::Text] = e.Additional_ [AN::EF::FullText];
			else if (!e.Additional_ [AN::EF::ExtendedText].toString ().isEmpty ())
				e.Additional_ [EF::Text] = e.Additional_ [AN::EF::ExtendedText];
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
			if (key.startsWith ("org.LC.AdvNotifications."_ql))
				i = e.Additional_.erase (i);
			else
				++i;
		}

		static const auto advSuffix = "+advanced"_ql;
		if (e.Mime_.endsWith (advSuffix))
			e.Mime_.remove (advSuffix);

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

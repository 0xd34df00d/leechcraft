/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "alertdispatcher.h"
#include <QtDebug>
#include <libtorrent/alert.hpp>
#include <libtorrent/session.hpp>

namespace LC::BitTorrent
{
	AlertDispatcher::AlertDispatcher (libtorrent::session& session)
	: Session_ { session }
	{
	}

	void AlertDispatcher::Swallow (int alertType, bool logging)
	{
		Handlers_ [alertType].push_back ([logging] (const libtorrent::alert&) { return logging; });
	}

	void AlertDispatcher::PollAlerts ()
	{
		std::vector<libtorrent::alert*> alerts;
		Session_.pop_alerts (&alerts);
		for (const auto alert : alerts)
			DispatchAlert (*alert);
	}

	void AlertDispatcher::DispatchAlert (const libtorrent::alert& alert) const
	{
		const auto& handlers = Handlers_ [alert.type ()];
		const auto& tempHandlers = TempHandlers_ [alert.type ()];
		if (handlers.empty () && tempHandlers.empty ())
		{
			qDebug () << "<libtorrent> unhandled alert:" << alert.type () << alert.message ().c_str ();
			return;
		}

		bool log = true;
		for (const auto& handler : tempHandlers)
			log = handler (alert) && log;
		for (const auto& handler : handlers)
			log = handler (alert) && log;

		if (log)
			qDebug () << "<libtorrent>" << alert.type () << alert.message ().c_str ();
	}
}

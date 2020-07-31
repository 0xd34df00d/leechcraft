/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverupdater.h"
#include <Wt/WApplication.h>
#include <Wt/WServer.h>

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	ServerUpdater::ServerUpdater (Wt::WApplication *app)
	: App_ { app }
	, Session_ { app->sessionId () }
	{
	}

	void ServerUpdater::operator() () const
	{
		Wt::WServer::instance ()->post (Session_,
				[this] { App_->triggerUpdate (); });
	}

	void ServerUpdater::operator() (const std::function<void ()>& f) const
	{
		Wt::WServer::instance ()->post (Session_, f);
	}
}
}
}

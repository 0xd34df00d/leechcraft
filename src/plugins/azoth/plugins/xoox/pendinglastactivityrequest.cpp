/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendinglastactivityrequest.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	PendingLastActivityRequest::PendingLastActivityRequest (const QString& jid, QObject *parent)
	: QObject { parent }
	, Jid_ { jid }
	{
	}

	int PendingLastActivityRequest::GetTime () const
	{
		return Time_;
	}

	PendingLastActivityRequest::Context PendingLastActivityRequest::GetContext () const
	{
		if (Jid_.contains ('/'))
			return Context::Activity;
		else if (Jid_.contains ('@'))
			return Context::LastConnection;
		else
			return Context::Uptime;
	}

	void PendingLastActivityRequest::handleGotLastActivity (const QString& jid, int time)
	{
		if (jid != Jid_)
			return;

		Time_ = time;

		emit gotLastActivity ();
		deleteLater ();
	}
}
}
}

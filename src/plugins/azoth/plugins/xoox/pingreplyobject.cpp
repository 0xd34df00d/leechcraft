/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pingreplyobject.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	PingReplyObject::PingReplyObject (QObject *parent)
	: QObject { parent }
	{
	}

	int PingReplyObject::GetTimeout () const
	{
		return Timeout_;
	}

	void PingReplyObject::HandleReply (int msecs)
	{
		Timeout_ = msecs;
		emit replyReceived (msecs);
		deleteLater ();
	}
}
}
}

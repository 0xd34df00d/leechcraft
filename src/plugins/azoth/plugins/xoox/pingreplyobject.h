/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/ihavepings.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PingReplyObject : public QObject
						  , public IPendingPing
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IPendingPing)

		int Timeout_ = -1;
	public:
		PingReplyObject (QObject* = nullptr);

		int GetTimeout () const;

		void HandleReply (int msecs);
	signals:
		void replyReceived (int);
	};
}
}
}

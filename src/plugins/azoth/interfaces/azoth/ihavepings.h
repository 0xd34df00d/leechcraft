/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	class IPendingPing
	{
	public:
		virtual ~IPendingPing () {}

		virtual int GetTimeout () const = 0;
	protected:
		virtual void replyReceived (int timeout) = 0;
	};

	class IHavePings
	{
	public:
		virtual ~IHavePings () {}

		virtual QObject* Ping (const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IPendingPing,
		"org.LeechCraft.Azoth.IPendingPing/1.0")
Q_DECLARE_INTERFACE (LC::Azoth::IHavePings,
		"org.LeechCraft.Azoth.IHavePings/1.0")

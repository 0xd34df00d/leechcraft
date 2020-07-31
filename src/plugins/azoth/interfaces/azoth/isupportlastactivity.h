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
	class IPendingLastActivityRequest
	{
	public:
		enum class Context
		{
			Activity,
			LastConnection,
			Uptime
		};

		virtual ~IPendingLastActivityRequest () {}

		virtual int GetTime () const = 0;

		virtual Context GetContext () const = 0;
	protected:
		virtual void gotLastActivity () = 0;
	};

	class ISupportLastActivity
	{
	public:
		virtual ~ISupportLastActivity () {}

		virtual QObject* RequestLastActivity (QObject *entry, const QString& variant) = 0;

		virtual QObject* RequestLastActivity (const QString& humanReadableId) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IPendingLastActivityRequest,
		"org.LeechCraft.Azoth.IPendingLastActivityRequest/1.0")
Q_DECLARE_INTERFACE (LC::Azoth::ISupportLastActivity,
		"org.LeechCraft.Azoth.ISupportLastActivity/1.0")

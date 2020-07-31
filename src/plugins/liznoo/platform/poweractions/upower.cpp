/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "upower.h"
#include <QtConcurrentRun>
#include <QtDBus>
#include <util/sll/unreachable.h>

namespace LC
{
namespace Liznoo
{
namespace PowerActions
{
	namespace
	{
		QByteArray State2Method (Platform::State state)
		{
			switch (state)
			{
			case Platform::State::Suspend:
				return "Suspend";
			case Platform::State::Hibernate:
				return "Hibernate";
			}

			Util::Unreachable ();
		}
	}

	QFuture<bool> UPower::IsAvailable ()
	{
		return QtConcurrent::run ([]
				{
					QDBusInterface face ("org.freedesktop.UPower",
							"/org/freedesktop/UPower",
							"org.freedesktop.UPower",
							QDBusConnection::systemBus ());
					return face.isValid () &&
							face.property ("CanSuspend").isValid () &&
							face.property ("CanHibernate").isValid ();
				});
	}

	QFuture<Platform::QueryChangeStateResult> UPower::CanChangeState (State state)
	{
		return QtConcurrent::run ([state] () -> QueryChangeStateResult
				{
					QDBusInterface face ("org.freedesktop.UPower",
							"/org/freedesktop/UPower",
							"org.freedesktop.UPower",
							QDBusConnection::systemBus ());
					if (!face.isValid ())
						return { false, tr ("Cannot connect to UPower daemon.") };

					return { face.property ("Can" + State2Method (state)).toBool (), {} };
				});
	}

	void UPower::ChangeState (State state)
	{
		QDBusInterface face ("org.freedesktop.UPower",
				"/org/freedesktop/UPower",
				"org.freedesktop.UPower",
				QDBusConnection::systemBus ());

		face.call (QDBus::NoBlock, State2Method (state));
	}
}
}
}

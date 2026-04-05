/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "freebsd.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <dev/acpica/acpiio.h>
#include <util/sys/fdguard.h>
#include <util/threads/coro.h>

namespace LC::Liznoo::PowerActions
{
	Util::ContextTask<bool> FreeBSD::IsAvailable ()
	{
		// We don't have other backends for FreeBSD anyway.
		co_return true;
	}

	namespace
	{
		QString GetWriteErrnoMsg ()
		{
			return errno == EACCES ?
					FreeBSD::tr ("No permissions to write to %1. If you are in the %2 group, add "
						"%3 to %4 and run %5 to apply the required permissions to %1.")
						.arg ("<em>/dev/acpi</em>")
						.arg ("<em>wheel</em>")
						.arg ("<code>perm acpi 0664</code>")
						.arg ("<em>/etc/devfs.conf</em>")
						.arg ("<code>/etc/rc.d/devfs restart</code>") :
					FreeBSD::tr ("Unable to open %1 for writing: %2.")
						.arg ("<em>/dev/acpi</em>")
						.arg (errno);
		}
	}

	Util::ContextTask<Platform::Result> FreeBSD::CanChangeState (State)
	{
		if (Util::FDGuard { "/dev/acpi", O_WRONLY })
			co_return Success {};

		co_return Fail { GetWriteErrnoMsg () };
	}

	Util::ContextTask<Platform::Result> FreeBSD::ChangeState (State state)
	{
		const Util::FDGuard fd { "/dev/acpi", O_WRONLY };
		if (!fd)
		{
			qWarning () << "unable to open /dev/acpi for writing, errno is:" << errno;
			co_return Fail { GetWriteErrnoMsg () };
		}

		int sleep_state = -1;
		switch (state)
		{
		case State::Suspend:
			sleep_state = 3;
			break;
		case State::Hibernate:
			sleep_state = 4;
			break;
		}

		const auto res = ioctl (fd, ACPIIO_REQSLPSTATE, &sleep_state);
		if (res == -1)
		{
			qWarning () << "unable to perform ioctl, errno is:" << errno;
			co_return Fail { tr ("Unable to perform ioctl: %1.").arg (errno) };
		}

		co_return Success {};
	}
}

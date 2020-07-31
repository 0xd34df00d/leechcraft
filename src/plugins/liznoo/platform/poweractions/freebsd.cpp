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
#include <QMessageBox>
#include <util/sys/fdguard.h>
#include <util/threads/futures.h>

namespace LC
{
namespace Liznoo
{
namespace PowerActions
{
	QFuture<bool> FreeBSD::IsAvailable ()
	{
		// We don't have other backends for FreeBSD anyway.
		return Util::MakeReadyFuture (true);
	}

	QFuture<Platform::QueryChangeStateResult> FreeBSD::CanChangeState (Platform::State)
	{
		QFutureInterface<QueryChangeStateResult> iface;

		if (Util::FDGuard { "/dev/acpi", O_WRONLY })
		{
			const QueryChangeStateResult result { true, {} };
			iface.reportFinished (&result);
		}
		else
		{
			const auto& msg = errno == EACCES ?
					tr ("No permissions to write to %1. If you are in the %2 group, add "
						"%3 to %4 and run %5 to apply the required permissions to %1.")
						.arg ("<em>/dev/acpi</em>")
						.arg ("<em>wheel</em>")
						.arg ("<code>perm acpi 0664</code>")
						.arg ("<em>/etc/devfs.conf</em>")
						.arg ("<code>/etc/rc.d/devfs restart</code>") :
					tr ("Unable to open %1 for writing.")
						.arg ("<em>/dev/acpi</em>");
			const QueryChangeStateResult result { false, msg };
			iface.reportFinished (&result);
		}

		return iface.future ();
	}

	void FreeBSD::ChangeState (Platform::State state)
	{
		const Util::FDGuard fd { "/dev/acpi", O_WRONLY };
		if (!fd)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open /dev/acpi for writing, errno is:"
					<< errno;
			return;
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
		default:
			return;
		}

		const auto res = ioctl (fd, ACPIIO_REQSLPSTATE, &sleep_state);
		if (res == -1)
			qWarning () << Q_FUNC_INFO
					<< "unable to perform ioctl, errno is:"
					<< errno;
	}
}
}
}

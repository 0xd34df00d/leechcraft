/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "connector.h"
#include <QtDebug>

namespace LC
{
namespace Liznoo
{
namespace ConsoleKit
{
	Connector::Connector (QObject *parent)
	: ConnectorBase { "org.freedesktop.ConsoleKit", "CK", parent }
	{
		if (!TryAutostart ())
			return;

		if (!CheckSignals ("/org/freedesktop/ConsoleKit/Manager", { "\"PrepareForSleep\"" }))
		{
			qDebug () << Q_FUNC_INFO
					<< "no PrepareForSleep signal";
			return;
		}

		PowerEventsAvailable_ = SB_.connect ("org.freedesktop.ConsoleKit",
				"/org/freedesktop/ConsoleKit/Manager",
				"org.freedesktop.ConsoleKit.Manager",
				"PrepareForSleep",
				this,
				SLOT (handlePrepare (bool)));
	}

	void Connector::handlePrepare (bool active)
	{
		if (active)
			emit gonnaSleep (1000);
		else
			emit wokeUp ();
	}
}
}
}

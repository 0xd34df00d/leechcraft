/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entityconstants.h"
#include <QVariant>

namespace LC::EF
{
	const QString Priority { QStringLiteral ("Priority") };
	const QString NotificationActions { QStringLiteral ("NotificationActions") };
	const QString NotificationPixmap { QStringLiteral ("NotificationPixmap") };
	const QString NotificationTimeout { QStringLiteral ("NotificationTimeout") };
	const QString HandlingObject { QStringLiteral ("HandlingObject") };
	const QString UserVisibleName { QStringLiteral ("UserVisibleName") };
	const QString Text { QStringLiteral ("Text") };
	const QString Tags { QStringLiteral ("Tags") };

	namespace GlobalAction
	{
		const QString ActionID { QStringLiteral ("ActionID") };
		const QString Shortcut { QStringLiteral ("Shortcut") };
		const QString Receiver { QStringLiteral ("Receiver") };
		const QString Method { QStringLiteral ("Method") };
		const QString AltShortcuts { QStringLiteral ("AltShortcuts") };
	}

	namespace PowerState
	{
		const QString TimeLeft { QStringLiteral ("TimeLeft") };
	}
}

namespace LC::Mimes
{
#define MIME_NOTIFICATION "x-leechcraft/notification"
	const QString Notification { QStringLiteral (MIME_NOTIFICATION) };
	const QString NotificationRuleCreate { QStringLiteral (MIME_NOTIFICATION "-rule-create") };

	const QString GlobalActionRegister { QStringLiteral ("x-leechcraft/global-action-register") };
	const QString GlobalActionUnregister { QStringLiteral ("x-leechcraft/global-action-unregister") };

	const QString PowerStateChanged { QStringLiteral ("x-leechcraft/power-state-changed") };

	const QString DataFilterRequest { QStringLiteral ("x-leechcraft/data-filter-request") };
}

namespace LC::PowerState
{
	const QVariant Sleeping { QByteArray { "Sleeping" } };
	const QVariant WokeUp { QByteArray { "WokeUp" } };
}

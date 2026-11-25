/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entityfields.h"

namespace LC::AN::EF
{
	const QString SenderID { QStringLiteral ("org.LC.AdvNotifications.SenderID") };
	const QString EventCategory { QStringLiteral ("org.LC.AdvNotifications.EventCategory") };
	const QString EventID { QStringLiteral ("org.LC.AdvNotifications.EventID") };
	const QString VisualPath { QStringLiteral ("org.LC.AdvNotifications.VisualPath") };
	const QString EventType { QStringLiteral ("org.LC.AdvNotifications.EventType") };
	const QString DeltaCount { QStringLiteral ("org.LC.AdvNotifications.DeltaCount") };
	const QString Count { QStringLiteral ("org.LC.AdvNotifications.Count") };

	const QString WindowIndex { QStringLiteral ("org.LC.AdvNotifications.WindowIndex") };

	const QString OpenConfiguration { QStringLiteral ("org.LC.AdvNotifications.OpenConfiguration") };
	const QString IsSingleShot { QStringLiteral ("org.LC.AdvNotifications.SingleShot") };
	const QString NotifyTransient { QStringLiteral ("org.LC.AdvNotifications.NotifyTransient") };
	const QString NotifyPersistent { QStringLiteral ("org.LC.AdvNotifications.NotifyPersistent") };
	const QString NotifyAudio { QStringLiteral ("org.LC.AdvNotifications.NotifyAudio") };

	const QString RuleID { QStringLiteral ("org.LC.AdvNotifications.RuleID") };
	const QString AssocColor { QStringLiteral ("org.LC.AdvNotifications.AssocColor") };
	const QString IsEnabled { QStringLiteral ("org.LC.AdvNotifications.IsEnabled") };
}

namespace LC
{
	const QString IgnoreSelf { QStringLiteral ("IgnoreSelf") };
	const QString AllowedSemantics { QStringLiteral ("AllowedSemantics") };
}

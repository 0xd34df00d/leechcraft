/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC::EF
{
	Q_DECL_IMPORT extern const QString Priority;
	Q_DECL_IMPORT extern const QString NotificationActions;
	Q_DECL_IMPORT extern const QString NotificationPixmap;
	Q_DECL_IMPORT extern const QString NotificationTimeout;
	Q_DECL_IMPORT extern const QString HandlingObject;
	Q_DECL_IMPORT extern const QString UserVisibleName;
	Q_DECL_IMPORT extern const QString Text;
	Q_DECL_IMPORT extern const QString Tags;
}

namespace LC::Mimes
{
	Q_DECL_IMPORT extern const QString Notification;
	Q_DECL_IMPORT extern const QString NotificationRuleCreate;

	Q_DECL_IMPORT extern const QString GlobalActionRegister;
	Q_DECL_IMPORT extern const QString GlobalActionUnregister;

	Q_DECL_IMPORT extern const QString PowerStateChanged;

	Q_DECL_IMPORT extern const QString DataFilterRequest;
}

namespace LC::GlobalAction
{
	Q_DECL_IMPORT extern const QString ActionID;
	Q_DECL_IMPORT extern const QString Shortcut;
	Q_DECL_IMPORT extern const QString Receiver;
	Q_DECL_IMPORT extern const QString Method;
	Q_DECL_IMPORT extern const QString AltShortcuts;
}

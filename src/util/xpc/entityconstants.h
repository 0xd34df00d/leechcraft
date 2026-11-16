/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include "xpcconfig.h"

class QVariant;

/** @brief The various Additional_ entity fields.
 */
namespace LC::EF
{
	UTIL_XPC_API extern const QString Priority;
	UTIL_XPC_API extern const QString NotificationActions;
	UTIL_XPC_API extern const QString NotificationPixmap;
	UTIL_XPC_API extern const QString NotificationTimeout;
	UTIL_XPC_API extern const QString HandlingObject;
	UTIL_XPC_API extern const QString UserVisibleName;
	UTIL_XPC_API extern const QString Text;
	UTIL_XPC_API extern const QString Tags;

	/** @brief Entity fields corresponding to global action registration.
	 *
	 * @sa LC::Mimes::GlobalActionRegister
	 */
	namespace GlobalAction
	{
		UTIL_XPC_API extern const QString ActionID;
		UTIL_XPC_API extern const QString Shortcut;
		UTIL_XPC_API extern const QString Receiver;
		UTIL_XPC_API extern const QString Method;
		UTIL_XPC_API extern const QString AltShortcuts;
	}

	/** @brief Entity fields corresponding to power state change events.
	 *
	 * @sa LC::Mimes::PowerStateChanged
	 */
	namespace PowerState
	{
		UTIL_XPC_API extern const QString TimeLeft;
	}
}

/** @brief The various well-known values of the inter-plugin Entity's MIME_ field.
 */
namespace LC::Mimes
{
	UTIL_XPC_API extern const QString Notification;
	UTIL_XPC_API extern const QString NotificationRuleCreate;

	/** @brief Registration of a global system-wide action.
	 *
	 * @sa LC::EF::GlobalAction
	 */
	UTIL_XPC_API extern const QString GlobalActionRegister;
	UTIL_XPC_API extern const QString GlobalActionUnregister;

	/** @brief Energy power state has changed.
	 *
	 * @sa LC::PowerState
	 * @sa LC::EF::PowerState
	 */
	UTIL_XPC_API extern const QString PowerStateChanged;

	UTIL_XPC_API extern const QString DataFilterRequest;
}

/** @brief The values of the Entity::Entity_ corresponding to LC::Mimes::PowerStateChanged.
 */
namespace LC::PowerState
{
	UTIL_XPC_API extern const QVariant Sleeping;
	UTIL_XPC_API extern const QVariant WokeUp;
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

class QVariant;

/** @brief The various Additional_ entity fields.
 */
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

	/** @brief Entity fields corresponding to global action registration.
	 *
	 * @sa LC::Mimes::GlobalActionRegister
	 */
	namespace GlobalAction
	{
		Q_DECL_IMPORT extern const QString ActionID;
		Q_DECL_IMPORT extern const QString Shortcut;
		Q_DECL_IMPORT extern const QString Receiver;
		Q_DECL_IMPORT extern const QString Method;
		Q_DECL_IMPORT extern const QString AltShortcuts;
	}

	/** @brief Entity fields corresponding to power state change events.
	 *
	 * @sa LC::Mimes::PowerStateChanged
	 */
	namespace PowerState
	{
		Q_DECL_IMPORT extern const QString TimeLeft;
	}
}

/** @brief The various well-known values of the inter-plugin Entity's MIME_ field.
 */
namespace LC::Mimes
{
	Q_DECL_IMPORT extern const QString Notification;
	Q_DECL_IMPORT extern const QString NotificationRuleCreate;

	/** @brief Registration of a global system-wide action.
	 *
	 * @sa LC::EF::GlobalAction
	 */
	Q_DECL_IMPORT extern const QString GlobalActionRegister;
	Q_DECL_IMPORT extern const QString GlobalActionUnregister;

	/** @brief Energy power state has changed.
	 *
	 * @sa LC::PowerState
	 * @sa LC::EF::PowerState
	 */
	Q_DECL_IMPORT extern const QString PowerStateChanged;

	Q_DECL_IMPORT extern const QString DataFilterRequest;
}

/** @brief The values of the Entity::Entity_ corresponding to LC::Mimes::PowerStateChanged.
 */
namespace LC::PowerState
{
	Q_DECL_IMPORT extern const QVariant Sleeping;
	Q_DECL_IMPORT extern const QVariant WokeUp;
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantMap>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
struct Entity;

namespace Laughty
{
	class ServerObject : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		uint32_t LastID_;
	public:
		ServerObject (ICoreProxy_ptr);

		QStringList GetCapabilities () const;

		uint Notify (const QString& app_name, uint replaces_id, const QString& app_icon,
				QString summary, QString body, const QStringList& actions,
				const QVariantMap& hints, uint expire_timeout);

		void CloseNotification (uint id);
	private:
		void HandleActions (Entity&, int, const QStringList&, const QVariantMap&);

		void HandleImages (Entity&, const QString&, const QVariantMap&);
		bool HandleImageData (Entity&, const QVariantMap&);
		bool HandleImagePath (Entity&, const QVariantMap&);
		bool HandleImageAppIcon (Entity&, const QString&);

		void HandleSounds (const QVariantMap&);
	signals:
		void NotificationClosed (uint id, uint reason);
		void ActionInvoked (uint id, const QString& action_key);
	};
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDBusAbstractAdaptor>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Laughty
{
	class ServerObject;

	class ServerAdaptor : public QDBusAbstractAdaptor
	{
		Q_OBJECT
		Q_CLASSINFO ("D-Bus Interface", "org.freedesktop.Notifications")

		ServerObject * const Server_;
		const ICoreProxy_ptr Proxy_;
	public:
		ServerAdaptor (ServerObject*, ICoreProxy_ptr);
	public slots:
		QStringList GetCapabilities () const;
		uint Notify (const QString& app_name, uint replaces_id, const QString& app_icon,
				const QString& summary, const QString& body, const QStringList& actions,
				const QVariantMap& hints, int expire_timeout);
		void CloseNotification (uint id);
		void GetServerInformation (QString& name, QString& vendor,
				QString& version, QString& spec_version) const;
	signals:
		void NotificationClosed (uint id, uint reason);
		void ActionInvoked (uint id, const QString& action_key);
	};
}
}

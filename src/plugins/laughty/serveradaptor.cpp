/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serveradaptor.h"
#include <QStringList>
#include "serverobject.h"

namespace LC
{
namespace Laughty
{
	ServerAdaptor::ServerAdaptor (ServerObject *obj, ICoreProxy_ptr proxy)
	: QDBusAbstractAdaptor (obj)
	, Server_ (obj)
	, Proxy_ (proxy)
	{
	}

	QStringList ServerAdaptor::GetCapabilities () const
	{
		return Server_->GetCapabilities ();
	}

	uint ServerAdaptor::Notify (const QString& app_name, uint replaces_id,
			const QString& app_icon, const QString& summary, const QString& body,
			const QStringList& actions, const QVariantMap& hints, int expire_timeout)
	{
		return Server_->Notify (app_name, replaces_id, app_icon,
				summary, body, actions, hints, expire_timeout);
	}

	void ServerAdaptor::CloseNotification (uint id)
	{
		Server_->CloseNotification (id);
	}

	void ServerAdaptor::GetServerInformation (QString& name,
			QString& vendor, QString& version, QString& spec_version) const
	{
		name = "Laughty";
		vendor = "LeechCraft";
		version = Proxy_->GetVersion ();
		spec_version = "1.2";
	}
}
}

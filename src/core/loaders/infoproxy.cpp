/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "infoproxy.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QIcon>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Loaders
{
	InfoProxy::InfoProxy (const QString& service)
	: Service_ { service }
	, IFace_ { new QDBusInterface { service, "/org/LeechCraft/Plugin" } }
	, Info_ { new QDBusInterface { service, "/org/LeechCraft/Info" } }
	{
	}

	void InfoProxy::SetProxy (ICoreProxy_ptr proxy)
	{
		Info_->call ("SetProxy", QVariant::fromValue (proxy));
	}

	void InfoProxy::Init (ICoreProxy_ptr proxy)
	{
		qDebug () << Q_FUNC_INFO;
		Info_->call ("Init", QVariant::fromValue (proxy));
		qDebug () << "done";
	}

	void InfoProxy::SecondInit ()
	{
		Info_->call ("SecondInit");
	}

	void InfoProxy::Release ()
	{
		Info_->call ("Release");
	}

	QByteArray InfoProxy::GetUniqueID () const
	{
		return QDBusReply<QByteArray> (Info_->call ("GetUniqueID")).value ();
	}

	QString InfoProxy::GetName () const
	{
		return QDBusReply<QString> (Info_->call ("GetName")).value ();
	}

	QString InfoProxy::GetInfo () const
	{
		return QDBusReply<QString> (Info_->call ("GetInfo")).value ();
	}

	QIcon InfoProxy::GetIcon () const
	{
		return QDBusReply<QIcon> (Info_->call ("GetIcon")).value ();
	}
}
}

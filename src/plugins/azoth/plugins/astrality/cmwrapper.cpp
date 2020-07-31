/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cmwrapper.h"
#include <util/sll/prelude.h>
#include <util/sll/functional.h>
#include <QtDebug>
#include <ConnectionManager>
#include <PendingReady>
#include "protowrapper.h"

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	CMWrapper::CMWrapper (const QString& cmName, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, CM_ (Tp::ConnectionManager::create (cmName))
	, Proxy_ (proxy)
	{
		connect (CM_->becomeReady (),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleCMReady (Tp::PendingOperation*)));
	}

	QList<QObject*> CMWrapper::GetProtocols () const
	{
		return Util::Map (ProtoWrappers_, Util::Upcast<QObject*>);
	}

	void CMWrapper::handleCMReady (Tp::PendingOperation *op)
	{
		if (op->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< CM_->name ()
					<< op->errorName ()
					<< op->errorMessage ();
			return;
		}

		qDebug () << Q_FUNC_INFO << CM_->name ();
		QList<QObject*> newProtoWrappers;
		for (const auto& proto : CM_->supportedProtocols ())
		{
			qDebug () << "has protocol" << proto;
			if (proto == "jabber" || proto == "irc")
				continue;

			auto pw = new ProtoWrapper (CM_, proto, Proxy_, this);
			ProtoWrappers_ << pw;
			newProtoWrappers << pw;
		}

		emit gotProtoWrappers (newProtoWrappers);
	}
}
}
}

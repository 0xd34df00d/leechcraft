/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <Types>
#include <ConnectionManager>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	class ProtoWrapper;

	class CMWrapper : public QObject
	{
		Q_OBJECT

		Tp::ConnectionManagerPtr CM_;
		QList<ProtoWrapper*> ProtoWrappers_;

		const ICoreProxy_ptr Proxy_;
	public:
		CMWrapper (const QString&, const ICoreProxy_ptr&, QObject* = 0);

		QList<QObject*> GetProtocols () const;
	private slots:
		void handleCMReady (Tp::PendingOperation*);
	signals:
		void gotProtoWrappers (QList<QObject*>);
	};
}
}
}

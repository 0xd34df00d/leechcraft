/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include <QFutureInterface>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include "../../batteryinfo.h"

namespace LC
{
namespace Liznoo
{
namespace Events
{
	class PlatformLayer : public QObject
	{
		Q_OBJECT

		QFutureInterface<bool> IsAvailable_;
	protected:
		const ICoreProxy_ptr Proxy_;
	public:
		PlatformLayer (const ICoreProxy_ptr&, QObject* = nullptr);

		QFuture<bool> IsAvailable ();
	protected slots:
		void setAvailable (bool);
	public slots:
		void emitGonnaSleep (int);
		void emitWokeUp ();
	};
}
}
}

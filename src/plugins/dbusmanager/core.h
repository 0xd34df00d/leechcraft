/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QDBusConnection>
#include <QStringList>
#include <interfaces/core/icoreproxy.h>
#include "general.h"
#include "tasks.h"

namespace LC
{
struct Entity;

namespace DBusManager
{
	class Core : public QObject
	{
		Q_OBJECT

		std::unique_ptr<QDBusConnection> Connection_;
		std::unique_ptr<General> General_;
		std::unique_ptr<Tasks> Tasks_;

		ICoreProxy_ptr Proxy_;

		Core ();
	public:
		static Core& Instance ();
		void Release ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
	private slots:
		void doDelayedInit ();
	};
}
}

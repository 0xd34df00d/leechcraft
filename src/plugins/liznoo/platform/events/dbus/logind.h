/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include "dbusplatformbase.h"

namespace LC::Liznoo::Events
{
	class Logind : public DBusPlatform<Logind>
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Liznoo::Events::Logind);

		Util::DBus::EndpointWithSignals Logind_;
	public:
		static const Config Config;

		explicit Logind (bool available, QObject* = nullptr);
	private:
		Util::Task<void> Inhibit ();
	};
}

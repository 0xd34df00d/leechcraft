/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sys/fdguard.h>
#include "batteryplatform.h"

class QTimer;

namespace LC
{
namespace Liznoo
{
namespace Battery
{
	class FreeBSDPlatform : public BatteryPlatform
	{
		Q_OBJECT

		QTimer * const Timer_;
		const Util::FDGuard ACPIfd_;
	public:
		FreeBSDPlatform (QObject* = nullptr);
	private slots:
		void update ();
	};
}
}
}

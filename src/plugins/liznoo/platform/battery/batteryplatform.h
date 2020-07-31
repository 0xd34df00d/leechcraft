/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "../../batteryinfo.h"

namespace LC
{
namespace Liznoo
{
namespace Battery
{
	class BatteryPlatform : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		void batteryInfoUpdated (Liznoo::BatteryInfo);
	};
}
}
}

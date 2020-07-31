/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "backend.h"
#include <QTimer>

namespace LC
{
namespace HotSensors
{
	Backend::Backend (QObject *parent)
	: QObject { parent }
	{
		auto timer = new QTimer { this };
#ifdef Q_OS_MAC
		timer->start (4000);
#else
		timer->start (1000);
#endif
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (update ()));
	}
}
}

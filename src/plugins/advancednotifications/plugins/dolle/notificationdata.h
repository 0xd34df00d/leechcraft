/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QColor>
#include <QHash>
#include <QString>

namespace LC
{
namespace AdvancedNotifications
{
namespace Dolle
{
	struct NotificationData
	{
		QColor Color_ = Qt::red;
		QHash<QString, int> Counts_;
		int Total_ = 0;
	};
}
}
}

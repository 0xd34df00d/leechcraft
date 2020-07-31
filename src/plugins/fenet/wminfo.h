/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>

namespace LC::Fenet
{
	struct WMInfo
	{
		QString Name_;
		QString Comment_;

		QStringList ExecNames_;

		QString Session_;

		bool SupportsCompositing_;
	};

	typedef QList<WMInfo> WMInfos_t;
}

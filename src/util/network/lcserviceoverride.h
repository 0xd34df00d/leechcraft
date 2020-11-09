/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include "networkconfig.h"

namespace LC::Util
{
	struct ServiceInfo
	{
		QString DefaultHost_;
		int DefaultPort_ = 0;
		QByteArray EnvPrefix_;

		bool UseSslByDefault_ = true;
	};

	UTIL_NETWORK_API QString GetServiceUrl (const ServiceInfo& serviceInfo, const QString& path);
}

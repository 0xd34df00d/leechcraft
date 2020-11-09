/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lcserviceoverride.h"

namespace LC
{
namespace Util
{
	namespace
	{
		QString GetHost (const ServiceInfo& info)
		{
			static const auto& env = qgetenv (info.EnvPrefix_ + "_SERVER_HOST");
			if (!env.isEmpty ())
				return env;

			return info.DefaultHost_;
		}

		int GetPort (const ServiceInfo& info)
		{
			static const auto& env = qgetenv (info.EnvPrefix_ + "_SERVER_PORT");
			if (!env.isEmpty ())
				return env.toInt ();

			return info.DefaultPort_;
		}

		QString GetProto (const ServiceInfo& info)
		{
			static const auto& env = qgetenv (info.EnvPrefix_ + "_NO_HTTPS");
			return env.isEmpty () && info.UseSslByDefault_ ? QStringLiteral ("https") : QStringLiteral ("http");
		}
	}

	QString GetServiceUrl (const ServiceInfo& info, const QString& path)
	{
		return QStringLiteral ("%1://%2:%3/%4")
				.arg (GetProto (info),
					  GetHost (info))
				.arg (GetPort (info))
				.arg (path);
	}
}
}

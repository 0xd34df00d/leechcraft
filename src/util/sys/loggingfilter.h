/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include "sysconfig.h"

namespace LC::Util
{
	class UTIL_SYS_API LoggingFilter final
	{
	public:
		using Categories = QHash<QLatin1String, QList<QtMsgType>>;
	private:
		struct Context;
		static QList<Context> ContextStack_;
	public:
		explicit LoggingFilter (const Categories& cats);
		~LoggingFilter ();

		LoggingFilter (const LoggingFilter&) = delete;
		LoggingFilter (LoggingFilter&&) = delete;
		LoggingFilter& operator= (const LoggingFilter&) = delete;
		LoggingFilter& operator= (LoggingFilter&&) = delete;
	private:
		static void Filter (QLoggingCategory*);
	};
}

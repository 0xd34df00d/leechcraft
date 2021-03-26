/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC::AnHero::CrashProcess
{
	struct AppInfo
	{
		int Signal_;
		uint64_t PID_;

		QString Path_;
		QString Version_;

		QString ExecLine_;

		bool SuggestRestart_;
	};

}

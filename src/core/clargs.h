/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVector>
#include <QStringList>

namespace LC
{
	struct Entity;
}

namespace LC::CL
{
	struct Args
	{
		QVector<Entity> Entities_;
		bool HelpRequested_;
		bool VersionRequested_;
		bool ListPluginsRequested_;

		bool NoResourceCaching_;
		bool NoSplashScreen_;
		bool Minimized_;

		bool SafeMode_;

		bool CatchExceptions_;
		bool NoLog_;
		bool Backtrace_;

		bool ClearSocket_;

		bool Multiprocess_;

		bool Restart_;

		QStringList Plugins_;
	};

	Args Parse (const QStringList& args, const QString& curDir);
	std::string GetHelp ();
}

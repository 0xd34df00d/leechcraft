/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QDebug>

namespace LC::Eleeminator
{
	struct ProcessInfo
	{
		int Pid_;
		QString Command_;
		QString CommandLine_;
		QList<ProcessInfo> Children_;
	};
}

QDebug operator<< (QDebug, const LC::Eleeminator::ProcessInfo&);

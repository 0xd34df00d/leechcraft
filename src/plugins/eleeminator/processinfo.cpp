/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "processinfo.h"

namespace LC
{
namespace Eleeminator
{
	void PrintPI (QDebug& debug, const ProcessInfo& info, int level = 0)
	{
		const int levelShift = 2;

		auto printShift = [&debug, level]
		{
			for (int i = 0; i < level; ++i)
				debug << " ";
		};

		printShift ();
		debug << "PI { Pid: " << info.Pid_ << "; command: " << info.Command_ << "; command line: " << info.CommandLine_ << "; children: " << info.Children_.size ();

		if (!info.Children_.isEmpty ())
		{
			debug << ":\n";
			for (const auto& child : info.Children_)
				PrintPI (debug, child, level + levelShift);
		}

		printShift ();
		debug << "}\n";
	}
}
}

QDebug operator<< (QDebug debug, const LC::Eleeminator::ProcessInfo& info)
{
	LC::Eleeminator::PrintPI (debug.nospace (), info);
	return debug.space ();
}

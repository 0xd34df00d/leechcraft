/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Kinotify
{
	class FSWinWatcher : public QObject
	{
		ICoreProxy_ptr Proxy_;
	public:
		FSWinWatcher (ICoreProxy_ptr, QObject* = 0);

		bool IsCurrentFS ();
	};
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/xpc/basehookinterconnector.h>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT
	public:
		PluginManager (QObject* = 0);

		virtual void AddPlugin (QObject*);
	};
}
}
}

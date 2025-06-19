/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/threads/workerthreadbase.h>

namespace LC
{
namespace Liznoo
{
	template<typename ConnT>
	class DBusThread : public Util::WorkerThread<ConnT>
	{
	public:
		using Util::WorkerThread<ConnT>::WorkerThread;
	};

	template<typename ConnT>
	using DBusThread_ptr = std::shared_ptr<DBusThread<ConnT>>;
}
}

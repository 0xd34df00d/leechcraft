/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <string>
#include <functional>

namespace Wt
{
	class WApplication;
}

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	class ServerUpdater
	{
		Wt::WApplication * const App_;
		const std::string Session_;
	public:
		ServerUpdater (Wt::WApplication*);

		void operator() () const;
		void operator() (const std::function<void ()>&) const;
	};
}
}
}

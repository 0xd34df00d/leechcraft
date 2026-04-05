/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "platform.h"
#include <IOKit/pwr_mgt/IOPMLib.h>

namespace LC::Liznoo::Battery
{
	class Mac : public Platform
	{
		CFRunLoopSourceRef PSEventsSource_;
	public:
		Mac ();
		~Mac ();
	private:
		void HandlePowerSourcesChanged ();
	};
}

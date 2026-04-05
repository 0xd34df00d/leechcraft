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

namespace LC::Liznoo::Events
{
	class Mac : public Platform
	{
		Q_OBJECT

		IONotificationPortRef NotifyPortRef_;
		io_object_t NotifierObject_;
		io_connect_t Port_;
	public:
		Mac (QObject* = 0);
		~Mac ();

		void Stop ();
		void IOCallback (io_service_t, natural_t, void*);
	};
}

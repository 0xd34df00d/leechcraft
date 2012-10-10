/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include "platformlayer.h"
#include <IOKit/pwr_mgt/IOPMLib.h>

namespace LeechCraft
{
namespace Liznoo
{
	class PlatformMac : public PlatformLayer
	{
		Q_OBJECT

		IONotificationPortRef NotifyPortRef_;
		io_object_t NotifierObject_;
		io_connect_t Port_;
	public:
		PlatformMac (QObject* = 0);
		~PlatformMac ();

		void Stop ();
		void IOCallback (io_service_t, natural_t, void*);
	};
}
}

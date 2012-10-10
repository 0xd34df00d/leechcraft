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

#include "platformmac.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <QTimer>
#include <mach/mach_port.h>
#include <mach/mach_interface.h>
#include <mach/mach_init.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>

namespace LeechCraft
{
namespace Liznoo
{
	namespace
	{
		void IOCallbackProxy (void *refCon, io_service_t service, natural_t messageType, void *messageArgument)
		{
			auto platform = static_cast<PlatformMac*> (refCon);
			platform->IOCallback (service, messageType, messageArgument);
		}
	}

	PlatformMac::PlatformMac (QObject *parent)
	: PlatformLayer (parent)
	, Port_ (IORegisterForSystemPower (this, &NotifyPortRef_, IOCallbackProxy, &NotifierObject_))
	{
		if (!Port_)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to register for system power";
			return;
		}

		CFRunLoopAddSource (CFRunLoopGetCurrent (),
				IONotificationPortGetRunLoopSource (NotifyPortRef_),
				kCFRunLoopCommonModes);
		QTimer::singleShot (100,
				this,
				SIGNAL (started ()));
	}

	PlatformMac::~PlatformMac ()
	{
		Stop ();
	}

	void PlatformMac::Stop ()
	{
		if (!Port_)
			return;

		CFRunLoopRemoveSource (CFRunLoopGetCurrent (),
				IONotificationPortGetRunLoopSource (NotifyPortRef_),
				kCFRunLoopCommonModes);
		IODeregisterForSystemPower (&NotifierObject_);
		IOServiceClose (Port_);
		IONotificationPortDestroy (NotifyPortRef_);
		Port_ = 0;
	}

	void PlatformMac::IOCallback (io_service_t service, natural_t messageType, void *messageArgument)
	{
		switch (messageType)
		{
		case kIOMessageCanSystemSleep:
			IOCancelPowerChange (Port_, reinterpret_cast<long> (messageArgument));
			break;
		case kIOMessageSystemWillSleep:
			EmitGonnaSleep (30000);
			break;
		case kIOMessageSystemHasPoweredOn:
			EmitWokeUp ();
			break;
		default:
			break;
		}
	}
}
}

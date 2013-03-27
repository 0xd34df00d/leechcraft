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
#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPSKeys.h>
#include <CoreFoundation/CFNumber.h>

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

		void PSCallbackProxy (void *refCon)
		{
			static_cast<PlatformMac*> (refCon)->powerSourcesChanged ();
		}
	}

	PlatformMac::PlatformMac (QObject *parent)
	: PlatformLayer (parent)
	, Port_ (IORegisterForSystemPower (this, &NotifyPortRef_, IOCallbackProxy, &NotifierObject_))
	, PSEventsSource_ (IOPSNotificationCreateRunLoopSource (PSCallbackProxy, this))
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
		CFRunLoopAddSource (CFRunLoopGetCurrent (),
				PSEventsSource_,
				kCFRunLoopCommonModes);
		QTimer::singleShot (100,
				this,
				SIGNAL (started ()));

		QTimer::singleShot (100,
				this,
				SLOT (powerSourcesChanged ()));
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
				PSEventsSource_,
				kCFRunLoopCommonModes);
		CFRelease (PSEventsSource_);

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

	namespace
	{
		template<typename T>
		struct Numeric2ID
		{
		};

		template<>
		struct Numeric2ID<int>
		{
			enum { Value = kCFNumberIntType };
		};

		template<>
		struct Numeric2ID<double>
		{
			enum { Value = kCFNumberDoubleType };
		};

		template<typename T>
		T GetNum (CFDictionaryRef dict, NSString *keyStr, T def)
		{
			auto numRef = static_cast<CFNumberRef> (CFDictionaryGetValue (dict, keyStr));

			if (!numRef)
				return def;

			int result = 0;
			CFNumberGetValue (numRef, Numeric2ID<T>::Value, &result);
			return result;
		}
	}

	void PlatformMac::powerSourcesChanged ()
	{
		auto info = IOPSCopyPowerSourcesInfo ();
		if (!info)
			return;

		auto sourcesList = IOPSCopyPowerSourcesList (info);
		if (!sourcesList)
		{
			CFRelease (info);
			return;
		}

		for (CFIndex i = 0; i < CFArrayGetCount (sourcesList); ++i)
		{
			auto dict = IOPSGetPowerSourceDescription (info, CFArrayGetValueAtIndex (sourcesList, i));

			auto getInt = [&dict] (NSString *key, int def)
				{ return GetNum<int> (dict, key, def); };
			auto getDouble = [&dict] (NSString *key, int def)
				{ return GetNum<double> (dict, key, def); };
			auto getString = [&dict] (NSString *key, const QString& def) -> QString
			{
				auto strRef = static_cast<CFStringRef> (CFDictionaryGetValue (dict, key));
				if (!strRef)
					return def;
				return QString::fromLatin1 (CFStringGetCStringPtr (strRef, 0));
			};

			const auto currentCapacity = getInt (@kIOPSCurrentCapacityKey, 0);
			const auto maxCapacity = getInt (@kIOPSMaxCapacityKey, 0);

			const BatteryInfo bi =
			{
				getString (@kIOPSHardwareSerialNumberKey, QString ()),
				static_cast<char> (maxCapacity ? 100 * currentCapacity / maxCapacity : 0),
				getInt (@kIOPSTimeToFullChargeKey, 0) * 60,
				getInt (@kIOPSTimeToEmptyKey, 0) * 60,
				getDouble (@kIOPSVoltageKey, 0)
			};

			emit batteryInfoUpdated (bi);
		}

		CFRelease (sourcesList);
		CFRelease (info);
	}
}
}

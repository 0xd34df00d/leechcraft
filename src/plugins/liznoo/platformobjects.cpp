/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "platformobjects.h"
#include <memory>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/either.h>
#include <util/threads/coro.h>
#include "platform/screen/screenplatform.h"
#include "platform/battery/batteryplatform.h"

#if defined(Q_OS_LINUX)
	#include "platform/battery/upowerplatform.h"
	#include "platform/events/dbus/dbusplatformbase.h"
	#include "platform/events/dbus/consolekit.h"
	#include "platform/events/dbus/logind.h"
	#include "platform/events/dbus/upower.h"
	#include "platform/poweractions/upower.h"
	#include "platform/screen/freedesktop.h"
#elif defined(Q_OS_WIN32)
	#include "platform/battery/winapiplatform.h"
	#include "platform/events/platformwinapi.h"
	#include "platform/winapi/fakeqwidgetwinapi.h"
#elif defined(Q_OS_FREEBSD)
	#include "platform/battery/freebsdplatform.h"
	#include "platform/events/platformfreebsd.h"
	#include "platform/poweractions/freebsd.h"
	#include "platform/screen/freedesktop.h"
#elif defined(Q_OS_MAC)
	#include "platform/battery/macplatform.h"
	#include "platform/events/platformmac.h"
#else
	#pragma message ("Unsupported system")
#endif

namespace LC::Liznoo
{
	namespace
	{
		template<typename T, typename F>
		std::shared_ptr<T> Select (std::initializer_list<std::shared_ptr<T>> options, F pred)
		{
			for (const auto& option : options)
				if (option && pred (*option))
					return option;
			return {};
		}
	}

	Util::ContextTask<void> PlatformObjects::Init ()
	{
#ifdef Q_OS_LINUX
		const auto upowerEvents = co_await Events::UPower::Create ();
		const auto ckEvents = co_await Events::ConsoleKit::Create ();
		const auto logindEvents = co_await Events::Logind::Create ();

		EventsPlatform_ = Select<Events::PlatformLayer> ({ upowerEvents, ckEvents, logindEvents },
				[] (const Events::PlatformLayer& platform) { return platform.IsAvailable (); });
		if (!EventsPlatform_)
			qWarning () << "no events platform";

		ScreenPlatform_ = new Screen::Freedesktop (this);
		BatteryPlatform_ = std::make_shared<Battery::UPowerPlatform> ();

		const auto upowerActions = std::make_shared<PowerActions::UPower> ();
		if (co_await upowerActions->IsAvailable ())
			PowerActPlatform_ = upowerActions;
#elifdef Q_OS_WIN32
		const auto widget = std::make_shared<WinAPI::FakeQWidgetWinAPI> ();

		EventsPlatform_ = std::make_shared<Events::PlatformWinAPI> (widget);
		BatteryPlatform_ = std::make_shared<Battery::WinAPIPlatform> (widget);
#elifdef Q_OS_FREEBSD
		EventsPlatform_ = std::make_shared<Events::PlatformFreeBSD> ();
		PowerActPlatform_ = std::make_shared<PowerActions::FreeBSD> ();
		BatteryPlatform_ = std::make_shared<Battery::FreeBSDPlatform> ();
		ScreenPlatform_ = new Screen::Freedesktop (this);
#elifdef Q_OS_MAC
		BatteryPlatform_ = std::make_shared<Battery::MacPlatform> ();
		EventsPlatform_ = std::make_shared<Events::PlatformMac> ();
#endif

		if (BatteryPlatform_)
			connect (BatteryPlatform_.get (),
					&Battery::BatteryPlatform::batteryInfoUpdated,
					this,
					&PlatformObjects::batteryInfoUpdated);
		else
			qWarning () << "battery backend is not available";
	}

	Util::ContextTask<std::unique_ptr<PlatformObjects>> PlatformObjects::Create ()
	{
		auto result = std::unique_ptr<PlatformObjects> { new PlatformObjects };
		co_await result->Init ();
		co_return result;
	}

	Util::ContextTask<PlatformObjects::ChangeStateResult_t> PlatformObjects::ChangeState (PowerActions::Platform::State state)
	{
		using enum ChangeStateFailed::Reason;

		if (!PowerActPlatform_)
			co_return { Util::AsLeft, ChangeStateFailed { Unavailable } };

		co_await Util::AddContextObject { *this };

		co_await co_await PowerActPlatform_->CanChangeState (state);
		co_await co_await PowerActPlatform_->ChangeState (state);
		co_return ChangeStateSucceeded {};
	}

	void PlatformObjects::ProhibitScreensaver (bool enable, const QString& id)
	{
		if (!ScreenPlatform_)
		{
			qWarning () << "screen platform layer unavailable, screensaver prohibiton won't work";
			return;
		}

		ScreenPlatform_->ProhibitScreensaver (enable, id);
	}

	bool PlatformObjects::EmitTestSleep ()
	{
		if (!EventsPlatform_)
		{
			qWarning () << "platform backend unavailable";
			return false;
		}

		EventsPlatform_->NotifyGonnaSleep (1000);
		return true;
	}

	bool PlatformObjects::EmitTestWakeup ()
	{
		if (!EventsPlatform_)
		{
			qWarning () << "platform backend unavailable";
			return false;
		}

		EventsPlatform_->NotifyWokeUp ();
		return true;
	}
}

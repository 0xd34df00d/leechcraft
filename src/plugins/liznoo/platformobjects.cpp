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
#include <util/threads/coro/inparallel.h>
#include "platform/screen/platform.h"
#include "platform/battery/platform.h"

#if defined(Q_OS_LINUX)
	#include "platform/battery/upower.h"
	#include "platform/events/dbus/dbusplatformbase.h"
	#include "platform/events/dbus/consolekit.h"
	#include "platform/events/dbus/logind.h"
	#include "platform/events/dbus/upower.h"
	#include "platform/poweractions/logind.h"
	#include "platform/poweractions/upower.h"
	#include "platform/screen/freedesktop.h"
#elif defined(Q_OS_WIN32)
	#include "platform/battery/windows.h"
	#include "platform/events/windows.h"
	#include "platform/winapi/fakeqwidgetwinapi.h"
#elif defined(Q_OS_FREEBSD)
	#include "platform/battery/freebsd.h"
	#include "platform/events/freebsd.h"
	#include "platform/poweractions/freebsd.h"
	#include "platform/screen/freedesktop.h"
#elif defined(Q_OS_MAC)
	#include "platform/battery/mac.h"
	#include "platform/events/mac.h"
#else
	#pragma message ("Unsupported system")
#endif

namespace LC::Liznoo
{
	namespace
	{
		template<typename T, typename F>
		Util::Task<std::shared_ptr<T>> Select (std::initializer_list<std::shared_ptr<T>> options, F pred)
		{
			for (const auto& option : options)
				if (option && co_await pred (*option))
					co_return option;
			co_return {};
		}
	}

	Util::ContextTask<void> PlatformObjects::Init ()
	{
#ifdef Q_OS_LINUX
		const auto [upowerEvents, ckEvents, logindEvents] = co_await Util::InParallel (Events::UPower::Create (),
				Events::ConsoleKit::Create (),
				Events::Logind::Create ());

		EventsPlatform_ = co_await Select<Events::Platform> ({ upowerEvents, ckEvents, logindEvents },
				[] (const Events::Platform& platform) -> Util::Task<bool> { co_return platform.IsAvailable (); });
		if (!EventsPlatform_)
			qWarning () << "no events platform";

		ScreenPlatform_ = new Screen::Freedesktop (this);
		BatteryPlatform_ = std::make_shared<Battery::UPower> ();

		PowerActPlatform_ = co_await Select<PowerActions::Platform> ({
					std::make_shared<PowerActions::Logind> (),
					std::make_shared<PowerActions::UPower> ()
				},
				[] (const PowerActions::Platform& platform) { return platform.IsAvailable (); });
#elifdef Q_OS_WIN32
		const auto widget = std::make_shared<Windows::FakeQWidgetWinAPI> ();

		EventsPlatform_ = std::make_shared<Events::Windows> (widget);
		BatteryPlatform_ = std::make_shared<Battery::Windows> (widget);
#elifdef Q_OS_FREEBSD
		EventsPlatform_ = std::make_shared<Events::FreeBSD> ();
		PowerActPlatform_ = std::make_shared<PowerActions::FreeBSD> ();
		BatteryPlatform_ = std::make_shared<Battery::FreeBSD> ();
		ScreenPlatform_ = new Screen::Freedesktop (this);
#elifdef Q_OS_MAC
		BatteryPlatform_ = std::make_shared<Battery::Mac> ();
		EventsPlatform_ = std::make_shared<Events::Mac> ();
#endif

		if (BatteryPlatform_)
			connect (BatteryPlatform_.get (),
					&Battery::Platform::batteryInfoUpdated,
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

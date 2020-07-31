/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "platformobjects.h"
#include <vector>
#include <memory>
#include <boost/optional.hpp>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/threads/futures.h>
#include <util/sll/delayedexecutor.h>
#include <util/sll/either.h>
#include "availabilitychecker.h"
#include "platform/screen/screenplatform.h"
#include "platform/battery/batteryplatform.h"

#if defined(Q_OS_LINUX)
	#include "platform/battery/upowerplatform.h"
	#include "platform/events/platformupowerlike.h"
	#include "platform/poweractions/pmutils.h"
	#include "platform/poweractions/upower.h"
	#include "platform/screen/freedesktop.h"
	#include "platform/common/dbusthread.h"
	#include "platform/upower/upowerconnector.h"
	#include "platform/logind/logindconnector.h"
	#include "platform/consolekit/connector.h"
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

namespace LC
{
namespace Liznoo
{
	namespace
	{
#if defined(Q_OS_LINUX)
		class LogindEventsChecker final : public IChecker<Events::PlatformLayer>
		{
			const ICoreProxy_ptr Proxy_;

			std::shared_ptr<DBusThread<Logind::LogindConnector>> Thread_;
		public:
			LogindEventsChecker (const ICoreProxy_ptr& proxy)
			: Proxy_ { proxy }
			{
			}

			QFuture<bool> Check () override
			{
				Thread_ = std::make_shared<DBusThread<Logind::LogindConnector>> ();
				Thread_->start (QThread::LowestPriority);
				return Thread_->ScheduleImpl (&Logind::LogindConnector::ArePowerEventsAvailable);
			}

			std::shared_ptr<Events::PlatformLayer> Make () override
			{
				return Events::MakeUPowerLike (Thread_, Proxy_);
			}
		};
#endif

		template<typename T>
		class PowerActionsChecker final : public IChecker<PowerActions::Platform>
		{
			std::shared_ptr<T> Platform_;
		public:
			QFuture<bool> Check () override
			{
				Platform_ = std::make_shared<T> ();
				return Platform_->IsAvailable ();
			}

			std::shared_ptr<PowerActions::Platform> Make () override
			{
				return Platform_;
			}
		};
	}

	PlatformObjects::PlatformObjects (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	{
#if defined(Q_OS_LINUX)
		const auto upowerThread = std::make_shared<DBusThread<UPower::UPowerConnector>> ();
		const auto ckThread = std::make_shared<DBusThread<ConsoleKit::Connector>> ();

		const auto eventsChecker = new AvailabilityChecker<Events::PlatformLayer>
		{
			MakePureChecker<Events::PlatformLayer> ([upowerThread]
					{
						return upowerThread->
								ScheduleImpl (&UPower::UPowerConnector::ArePowerEventsAvailable);
					},
					[upowerThread, this] { return Events::MakeUPowerLike (upowerThread, Proxy_); }),
			MakePureChecker<Events::PlatformLayer> ([ckThread]
					{
						return ckThread->
								ScheduleImpl (&ConsoleKit::Connector::ArePowerEventsAvailable);
					},
					[ckThread, this] { return Events::MakeUPowerLike (ckThread, Proxy_); }),
			std::make_unique<LogindEventsChecker> (proxy)
		};
		Util::Sequence (this, eventsChecker->GetResult ()) >>
				[this] (const auto& maybeEvents)
				{
					if (maybeEvents)
						EventsPlatform_ = *maybeEvents;
					else
						qWarning () << Q_FUNC_INFO
								<< "no events platform";
				};

		ScreenPlatform_ = new Screen::Freedesktop (this);
		BatteryPlatform_ = std::make_shared<Battery::UPowerPlatform> (upowerThread);

		const auto actionsChecker = new AvailabilityChecker<PowerActions::Platform>
		{
			std::make_unique<PowerActionsChecker<PowerActions::UPower>> (),
			std::make_unique<PowerActionsChecker<PowerActions::PMUtils>> ()
		};
		Util::Sequence (this, actionsChecker->GetResult ()) >>
				[this] (const auto& maybeActions)
				{
					if (maybeActions)
						PowerActPlatform_ = *maybeActions;
					else
						qWarning () << Q_FUNC_INFO
								<< "no actions platform";
				};

		upowerThread->start (QThread::LowestPriority);
		ckThread->start (QThread::LowestPriority);
#elif defined(Q_OS_WIN32)
		const auto widget = std::make_shared<WinAPI::FakeQWidgetWinAPI> ();

		EventsPlatform_ = std::make_shared<Events::PlatformWinAPI> (widget, Proxy_);
		BatteryPlatform_ = std::make_shared<Battery::WinAPIPlatform> (widget);
#elif defined(Q_OS_FREEBSD)
		EventsPlatform_ = std::make_shared<Events::PlatformFreeBSD> (Proxy_);
		PowerActPlatform_ = std::make_shared<PowerActions::FreeBSD> ();
		BatteryPlatform_ = std::make_shared<Battery::FreeBSDPlatform> ();
		ScreenPlatform_ = new Screen::Freedesktop (this);
#elif defined(Q_OS_MAC)
		BatteryPlatform_ = std::make_shared<Battery::MacPlatform> ();
		EventsPlatform_ = std::make_shared<Events::PlatformMac> (Proxy_);
#endif

		if (BatteryPlatform_)
			connect (BatteryPlatform_.get (),
					SIGNAL (batteryInfoUpdated (Liznoo::BatteryInfo)),
					this,
					SIGNAL (batteryInfoUpdated (Liznoo::BatteryInfo)));
		else
			qWarning () << Q_FUNC_INFO
					<< "battery backend is not available";

	}

	QFuture<PlatformObjects::ChangeStateResult_t> PlatformObjects::ChangeState (PowerActions::Platform::State state)
	{
		if (!PowerActPlatform_)
			return Util::MakeReadyFuture (ChangeStateResult_t::Left ({
						ChangeStateFailed::Reason::Unavailable,
						{}
					}));

		return Util::Sequence (this, PowerActPlatform_->CanChangeState (state)) >>
				[state, this] (const PowerActions::Platform::QueryChangeStateResult& res)
				{
					if (res.CanChangeState_)
					{
						PowerActPlatform_->ChangeState (state);
						return Util::MakeReadyFuture (ChangeStateResult_t::Right ({}));
					}
					else
						return Util::MakeReadyFuture (ChangeStateResult_t::Left ({
										ChangeStateFailed::Reason::PlatformFailure,
										res.Reason_
								}));
				};
	}

	void PlatformObjects::ProhibitScreensaver (bool enable, const QString& id)
	{
		if (!ScreenPlatform_)
		{
			qWarning () << Q_FUNC_INFO
					<< "screen platform layer unavailable, screensaver prohibiton won't work";
			return;
		}

		ScreenPlatform_->ProhibitScreensaver (enable, id);
	}

	bool PlatformObjects::EmitTestSleep ()
	{
		if (!EventsPlatform_)
		{
			qWarning () << Q_FUNC_INFO
					<< "platform backend unavailable";
			return false;
		}

		EventsPlatform_->emitGonnaSleep (1000);
		return true;
	}

	bool PlatformObjects::EmitTestWakeup ()
	{
		if (!EventsPlatform_)
		{
			qWarning () << Q_FUNC_INFO
					<< "platform backend unavailable";
			return false;
		}

		EventsPlatform_->emitWokeUp ();
		return true;
	}
}
}

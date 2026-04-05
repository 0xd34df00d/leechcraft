/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <util/threads/coro/taskfwd.h>
#include <interfaces/core/icoreproxyfwd.h>
#include "platform/poweractions/platform.h"

namespace LC::Liznoo
{
	namespace Events
	{
		class PlatformLayer;
	}

	namespace Screen
	{
		class ScreenPlatform;
	}

	namespace Battery
	{
		class BatteryPlatform;
	}

	struct BatteryInfo;

	class PlatformObjects : public QObject
	{
		Q_OBJECT

		std::shared_ptr<Events::PlatformLayer> EventsPlatform_;
		Screen::ScreenPlatform *ScreenPlatform_= nullptr;
		std::shared_ptr<PowerActions::Platform> PowerActPlatform_;
		std::shared_ptr<Battery::BatteryPlatform> BatteryPlatform_;

		explicit PlatformObjects () = default;
		Util::ContextTask<void> Init ();
	public:
		static Util::ContextTask<std::unique_ptr<PlatformObjects>> Create ();

		struct ChangeStateSucceeded {};
		struct ChangeStateFailed
		{
			enum class Reason : std::uint8_t
			{
				Unavailable,
				PlatformFailure,
				Other
			} Reason_;

			QString ReasonString_;

			explicit ChangeStateFailed (Reason reason)
			: Reason_ { reason }
			{
			}

			explicit ChangeStateFailed (PowerActions::Platform::Fail fail)
			: Reason_ { Reason::PlatformFailure }
			, ReasonString_ { fail.Reason_ }
			{
			}
		};
		using ChangeStateResult_t = Util::Either<ChangeStateFailed, ChangeStateSucceeded>;
		Util::ContextTask<ChangeStateResult_t> ChangeState (PowerActions::Platform::State);
		void ProhibitScreensaver (bool prohibit, const QString& id);

		bool EmitTestSleep ();
		bool EmitTestWakeup ();
	signals:
		void batteryInfoUpdated (const Liznoo::BatteryInfo&);
	};
}

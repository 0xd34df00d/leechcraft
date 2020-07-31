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
#include <util/sll/eitherfwd.h>
#include <util/sll/void.h>
#include <interfaces/core/icoreproxyfwd.h>
#include "platform/poweractions/platform.h"

template<typename>
class QFuture;

namespace LC
{
namespace Liznoo
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

		const ICoreProxy_ptr Proxy_;

		std::shared_ptr<Events::PlatformLayer> EventsPlatform_;
		Screen::ScreenPlatform *ScreenPlatform_= nullptr;
		std::shared_ptr<PowerActions::Platform> PowerActPlatform_;
		std::shared_ptr<Battery::BatteryPlatform> BatteryPlatform_;
	public:
		PlatformObjects (const ICoreProxy_ptr& proxy, QObject* = nullptr);

		struct ChangeStateSucceeded {};
		struct ChangeStateFailed
		{
			enum class Reason
			{
				Unavailable,
				PlatformFailure,
				Other
			} Reason_;

			QString ReasonString_;
		};
		using ChangeStateResult_t = Util::Either<ChangeStateFailed, ChangeStateSucceeded>;
		QFuture<ChangeStateResult_t> ChangeState (PowerActions::Platform::State);
		void ProhibitScreensaver (bool prohibit, const QString& id);

		bool EmitTestSleep ();
		bool EmitTestWakeup ();
	signals:
		void batteryInfoUpdated (const Liznoo::BatteryInfo&);
	};
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "availability.h"
#include <QNetworkInformation>
#include <QtDebug>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include "xmlsettingsmanager.h"

namespace LC::Aggregator::detail
{
	using namespace std::chrono_literals;

	namespace
	{
		struct Allowed {};
		struct Metered {};
		using Denied = std::variant<QNetworkInformation::Reachability, Metered>;

		Util::Either<Denied, Allowed> CheckNetworkInformation (const NetworkCheckContext& context)
		{
			auto info = QNetworkInformation::instance ();
			if (!info)
			{
				QNetworkInformation::loadDefaultBackend ();
				info = QNetworkInformation::instance ();
				if (!info)
					return Allowed {};
			}

			using enum QNetworkInformation::Reachability;
			switch (const auto reachability = info->reachability ())
			{
			case Unknown:
			case Online:
				break;
			case Disconnected:
			case Local:
			case Site:
				if (XmlSettingsManager::Instance ().property (context.ReachabilityProp_).toBool ())
					return Util::Left { reachability };
				break;
			}

			if (info->isMetered () && !XmlSettingsManager::Instance ().property (context.MeteredProp_).toBool ())
				return Util::Left { Metered {} };

			return Allowed {};
		}

		void DumpReason (QDebug& dbg, const Denied& denied)
		{
			using Reachability = QNetworkInformation::Reachability;
			return Util::Visit (denied,
					[&] (Metered) { dbg << "metered connection"; },
					[&] (Reachability reach) { dbg << "reachability is" << reach; });
		}
	}

	bool IsNetworkAllowed (const NetworkCheckContext& context, std::optional<Clock::time_point>& lastLog)
	{
		const auto info = CheckNetworkInformation (context);
		return Util::Visit (info,
				[] (Allowed) { return true; },
				[&] (Denied reason)
				{
					if (const auto now = Clock::now ();
						!lastLog || now - *lastLog > 30min)
					{
						lastLog.emplace (now);
						DumpReason (qWarning () << context.Action_ << "denied:", reason);
					}
					return false;
				});
	}
}

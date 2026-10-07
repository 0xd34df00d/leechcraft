/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <chrono>
#include <optional>
#include <string_view>
#include <util/sll/ctstring.h>

namespace LC::Aggregator
{
	inline constexpr auto FeedsUpdate = "feeds update"_ct;

	namespace detail
	{
		struct NetworkCheckContext
		{
			std::string_view Action_;

			const char *ReachabilityProp_;
			const char *MeteredProp_;
		};

		consteval NetworkCheckContext GetActionContext (std::string_view action)
		{
			if (action == FeedsUpdate)
				return
				{
					.Action_ = FeedsUpdate,
					.ReachabilityProp_ = "UpdateRespectReachability",
					.MeteredProp_ = "UpdateOnMetered",
				};

			throw std::runtime_error { "unknown action" };
		}

		using Clock = std::chrono::steady_clock;

		bool IsNetworkAllowed (const NetworkCheckContext& context, std::optional<Clock::time_point>& lastLog);
	}

	template<Util::CtString Action>
	bool IsNetworkAllowed ()
	{
		static std::optional<detail::Clock::time_point> lastLogTime {};

		constexpr auto context = detail::GetActionContext (Action);
		return detail::IsNetworkAllowed (context, lastLogTime);
	}
}

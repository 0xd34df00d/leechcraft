/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC::Util
{
	template<typename R, template<typename> typename... Extensions>
	class Task;

	template<typename>
	struct ContextExtension;

	/** @brief A Task cancelled along with the CoroContext it registers with.
	 *
	 * Every `co_await` in it checks the registered contexts, see AddContext
	 * and CoroContext for the contract.
	 */
	template<typename R = void>
	using ContextTask = Task<R, ContextExtension>;

	template<typename>
	struct SharedTaskExtension;

	template<typename R = void>
	using SharedTask = Task<R, SharedTaskExtension>;

	template<typename R = void>
	using SharedContextTask = Task<R, ContextExtension, SharedTaskExtension>;

	struct AwaiterCPO {};
}

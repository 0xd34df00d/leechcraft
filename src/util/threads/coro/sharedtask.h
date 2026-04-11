/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <QList>
#include <QtDebug>
#include "task.h"
#include "context.h"

namespace LC::Util
{
	template<typename>
	struct SharedTaskExtension
	{
		static constexpr bool IsAwaiterHandler = true;
		static constexpr bool IsResumeValueHandler = true;

		QList<std::coroutine_handle<>> Awaiters_;

		void AddAwaiter (std::coroutine_handle<> outerTask)
		{
			Awaiters_ << outerTask;
		}

		void RemoveAwaiter (std::coroutine_handle<> outerTask) noexcept
		{
			if (!Awaiters_.removeOne (outerTask))
				qFatal () << "awaiter not found";
		}

		auto GetAwaiters ()
		{
			return this->Awaiters_;
		}

		template<typename R>
		static const R& ResumeValue (R& ret) noexcept
		{
			return ret;
		}
	};

	template<typename T = void>
	using SharedTask = Task<T, SharedTaskExtension>;

	template<typename T = void>
	using SharedContextTask = Task<T, SharedTaskExtension, ContextExtension>;
}

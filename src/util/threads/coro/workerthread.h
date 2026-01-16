/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QThread>
#include "../coro.h"
#include "metamethod.h"

namespace LC::Util::Coro
{
	namespace detail
	{
		template<typename... Args>
		using Head = Args... [0];

		template<typename... Args>
		using SafeHead = Head<Args..., struct Dummy>;
	}

	class WorkerThreadBase : public QObject
	{
	protected:
		QThread Thread_;
	public:
		struct Config
		{
			QAnyStringView ThreadName_ = {};
			QThread::QualityOfService QOS_ = QThread::QualityOfService::Eco;
			QThread::Priority Priority_ = QThread::LowPriority;
		};

		explicit WorkerThreadBase (const Config& config)
		{
			if (!config.ThreadName_.isEmpty ())
				Thread_.setObjectName (config.ThreadName_);
			Thread_.setServiceLevel (config.QOS_);
			Thread_.start (config.Priority_);
		}

		~WorkerThreadBase () override
		{
			Thread_.quit ();
			Thread_.wait ();
		}
	};

	template<typename T>
	class WorkerThread : public WorkerThreadBase
	{
	protected:
		T Worker_;
	public:
		WorkerThread (const WorkerThread& thread) = delete;
		WorkerThread (WorkerThread&& thread) = delete;
		WorkerThread& operator= (const WorkerThread& thread) = delete;
		WorkerThread& operator= (WorkerThread&& thread) = delete;

		template<typename... Args>
			requires (!std::is_same_v<std::decay_t<detail::SafeHead<Args...>>, Config>)
		explicit WorkerThread (Args&&... args)
		: WorkerThread { Config {}, std::forward<Args> (args)... }
		{
		}

		template<typename... Args>
			requires std::constructible_from<T, Args&&...>
		explicit WorkerThread (const Config& config, Args&&... args)
		: WorkerThreadBase { config }
		, Worker_ { std::forward<Args> (args)... }
		{
			Worker_.moveToThread (&Thread_);
		}

		template<typename... Args>
			requires std::constructible_from<T, Args&&..., WorkerThread&>
		explicit WorkerThread (const Config& config, Args&&... args)
		: WorkerThreadBase { config }
		, Worker_ { std::forward<Args> (args)..., *this }
		{
			Worker_.moveToThread (&Thread_);
		}

		~WorkerThread () override
		{
			QMetaObject::invokeMethod (&Worker_,
					[this, thread = thread ()] { Worker_.moveToThread (thread); },
					Qt::BlockingQueuedConnection);
		}

		template<typename F, typename... Args, typename R = std::invoke_result_t<F, T*, Args...>>
		ContextTask<R> Run (F&& f, Args&&... args)
		{
			co_return co_await Util::MetaMethod (Worker_, std::forward<F> (f), std::forward<Args> (args)...);
		}
	};
}

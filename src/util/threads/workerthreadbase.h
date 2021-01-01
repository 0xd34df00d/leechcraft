/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <atomic>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QFutureInterface>
#include <QFuture>
#include <QList>
#include "futures.h"
#include "threadsconfig.h"

namespace LC::Util
{
	class UTIL_THREADS_API WorkerThreadBase : public QThread
	{
		Q_OBJECT

		std::atomic_bool IsPaused_ { false };

		QMutex FunctionsMutex_;
		QList<std::function<void ()>> Functions_;
	public:
		using QThread::QThread;

		void SetPaused (bool);

		template<typename F>
		QFuture<std::result_of_t<F ()>> ScheduleImpl (F func)
		{
			QFutureInterface<std::result_of_t<F ()>> iface;
			iface.reportStarted ();

			auto reporting = [func, iface] () mutable
			{
				ReportFutureResult (iface, func);
			};

			{
				QMutexLocker locker { &FunctionsMutex_ };
				Functions_ << reporting;
			}

			emit rotateFuncs ();

			return iface.future ();
		}

		template<typename F, typename... Args>
		QFuture<std::result_of_t<F (Args...)>> ScheduleImpl (F f, Args&&... args)
		{
			return ScheduleImpl ([f, args...] () mutable { return std::invoke (f, args...); });
		}

		virtual size_t GetQueueSize ();
	protected:
		void run () final;

		virtual void Initialize () = 0;
		virtual void Cleanup () = 0;
	private:
		void RotateFuncs ();
	signals:
		void rotateFuncs ();
	};

	namespace detail
	{
		template<typename WorkerType>
		struct InitializerBase
		{
			virtual std::unique_ptr<WorkerType> Initialize () = 0;

			virtual ~InitializerBase () = default;
		};

		template<typename WorkerType, typename... Args>
		struct Initializer final : InitializerBase<WorkerType>
		{
			std::tuple<Args...> Args_;

			Initializer (std::tuple<Args...>&& tuple)
			: Args_ { std::move (tuple) }
			{
			}

			std::unique_ptr<WorkerType> Initialize () override
			{
				return std::apply ([] (auto&&... args) { return std::make_unique<WorkerType> (std::forward<Args> (args)...); }, Args_);
			}
		};

		template<typename WorkerType>
		struct Initializer<WorkerType> final : InitializerBase<WorkerType>
		{
			std::unique_ptr<WorkerType> Initialize () override
			{
				return std::make_unique<WorkerType> ();
			}
		};
	}

	template<typename WorkerType>
	class WorkerThread : public WorkerThreadBase
	{
		std::atomic_bool IsAutoQuit_ { false };
		unsigned long QuitWait_ = 2000;
	protected:
		using W = WorkerType;

		std::unique_ptr<WorkerType> Worker_;

		std::unique_ptr<detail::InitializerBase<WorkerType>> Initializer_;
	public:
		WorkerThread (QObject *parent = nullptr)
		: WorkerThreadBase { parent }
		, Initializer_ { std::make_unique<detail::Initializer<WorkerType>> () }
		{
		}

		template<typename... Args>
		WorkerThread (QObject *parent, const Args&... args)
		: WorkerThreadBase { parent }
		, Initializer_ { std::make_unique<detail::Initializer<WorkerType, std::decay_t<Args>...>> (std::tuple<std::decay_t<Args>...> { args... }) }
		{
		}

		template<
				typename Head,
				typename... Rest,
				typename = std::enable_if_t<
						!std::is_base_of<QObject, std::remove_pointer_t<std::decay_t<Head>>>::value
					>
			>
		WorkerThread (const Head& head, const Rest&... rest)
		: WorkerThread { static_cast<QObject*> (nullptr), head, rest... }
		{
		}

		~WorkerThread ()
		{
			if (!IsAutoQuit_)
				return;

			quit ();
			wait (QuitWait_);

			if (isRunning ())
				qWarning () << Q_FUNC_INFO
						<< "thread is still running";
		}

		void SetAutoQuit (bool autoQuit)
		{
			IsAutoQuit_ = autoQuit;
		}

		void SetQuitWait (unsigned long wait)
		{
			QuitWait_ = wait;
		}

		using WorkerThreadBase::ScheduleImpl;

		template<typename F, typename... Args>
		QFuture<std::result_of_t<F (WorkerType*, Args...)>> ScheduleImpl (F f, Args&&... args)
		{
			const auto fWrapped = [f, this] (auto... args) mutable { return std::invoke (f, Worker_.get (), args...); };
			return WorkerThreadBase::ScheduleImpl (fWrapped, std::forward<Args> (args)...);
		}
	protected:
		void Initialize () override
		{
			Worker_ = Initializer_->Initialize ();

			Initializer_.reset ();
		}

		void Cleanup () override
		{
			Worker_.reset ();
		}
	};
}

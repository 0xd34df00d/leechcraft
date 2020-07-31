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
#include <util/sll/visitor.h>
#include "accountthread.h"

namespace LC
{
namespace Snails
{
	class Account;
	class Storage;

	using AccountThread_ptr = std::shared_ptr<AccountThread>;

	enum class TaskPriority;

	class ThreadPool : public QObject
	{
		Account * const Acc_;
		Storage * const Storage_;

		QList<AccountThread_ptr> ExistingThreads_;

		bool HitLimit_ = false;
		bool CheckingNext_ = false;

		QList<std::function<void (AccountThread*)>> Scheduled_;

		QList<std::function<void (AccountThread*)>> ThreadInitializers_;
	public:
		ThreadPool (Account*, Storage*, QObject* = nullptr);

		QFuture<EitherInvokeError_t<Util::Void>> TestConnectivity ();

		template<typename F, typename... Args>
		auto Schedule (TaskPriority prio, const F& func, const Args&... args)
		{
			QFutureInterface<WrapFunctionType_t<F, Args...>> iface;

			auto runner = [=] (AccountThread *thread) mutable
					{
						iface.reportStarted ();
						PerformScheduledFunc (thread, iface, prio, func, args...);
					};

			switch (prio)
			{
			case TaskPriority::High:
				Scheduled_.prepend (runner);
				break;
			case TaskPriority::Low:
				Scheduled_.append (runner);
				break;
			}

			RunThreads ();

			return iface.future ();
		}

		template<typename F, typename... Args>
		void AddThreadInitializer (const F& func, const Args&... args)
		{
			auto runner = [=] (AccountThread *thread) { std::invoke (func, thread, args...); };
			ThreadInitializers_ << runner;

			for (const auto& thread : ExistingThreads_)
				runner (thread.get ());
		}
	private:
		template<typename FutureInterface, typename F, typename... Args>
		void PerformScheduledFunc (AccountThread *thread, FutureInterface iface, TaskPriority prio, const F& func, const Args&... args)
		{
			Util::Sequence (nullptr, thread->Schedule (prio, func, args...)) >>
					[=] (auto result) mutable
					{
						if (result.IsRight ())
						{
							iface.reportFinished (&result);
							return;
						}

						Util::Visit (result.GetLeft (),
								[=, &iface] (const vmime::exceptions::authentication_error& e)
								{
									const auto& respStr = QString::fromStdString (e.response ());
									if (respStr.contains ("simultaneous"))
									{
										qWarning () << "Snails::ThreadPool::PerformScheduledFunc():"
												<< "seems like a thread has died, rescheduling...";
										HandleThreadOverflow (thread);
										PerformScheduledFunc (GetNextThread (), iface, prio, func, args...);
									}
									else
										iface.reportFinished (&result);
								},
								[=, &iface] (auto)
								{
									iface.reportFinished (&result);
								});
					};
		}

		void RunThreads ();

		AccountThread_ptr CreateThread ();

		void RunScheduled (AccountThread*);
		AccountThread* GetNextThread ();

		void HandleThreadOverflow (AccountThread*);
		void HandleThreadOverflow (const AccountThread_ptr&);
	};
}
}

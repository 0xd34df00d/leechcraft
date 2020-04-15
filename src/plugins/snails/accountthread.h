/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <thread>
#include <atomic>
#include <functional>
#include <QThread>
#include <QFuture>
#include <util/sll/either.h>
#include <util/sll/typelist.h>
#include <util/sll/typelevel.h>
#include <util/threads/futures.h>
#include <util/threads/workerthreadbase.h>
#include <vmime/security/cert/certificateException.hpp>
#include "accountthreadfwd.h"
#include "common.h"

namespace LC
{
namespace Snails
{
	class Account;
	class Storage;
	class AccountThreadWorker;

	class GenericExceptionWrapper
	{
		std::string Msg_;
	public:
		GenericExceptionWrapper (const std::exception_ptr&);

		const char* what () const noexcept;
	};

	namespace detail
	{
		const auto MaxRecLevel = 3;

		void ReconnectATW (AccountThreadWorker*);

		template<typename Result, typename F>
		class ExceptionsHandler
		{
			F&& F_;
			const int MaxRetries_;

			int RecLevel_ = 0;

			AccountThreadWorker * const W_;
		public:
			ExceptionsHandler (AccountThreadWorker *w, F&& f, int maxRetries)
			: F_ { std::forward<F> (f) }
			, MaxRetries_ { maxRetries }
			, W_ { w }
			{
			}

			template<typename Ex = std::exception>
			Result operator() (const Ex& ex = {})
			{
				if (RecLevel_)
				{
					using namespace std::chrono_literals;
					const auto timeout = RecLevel_ * 3000ms + 1000ms;
					qWarning () << "Snails::detail::ExceptionsHandler::operator():"
							<< "sleeping for"
							<< timeout.count ()
							<< "and retrying for the"
							<< RecLevel_
							<< "time after getting an exception:"
							<< ex.what ();

					std::this_thread::sleep_for (timeout);
				}

				if (RecLevel_ == MaxRetries_)
				{
					qWarning () << "Snails::detail::ExceptionsHandler::operator():"
							<< "giving up after"
							<< RecLevel_
							<< "retries:"
							<< ex.what ();

					if constexpr (Util::HasType<std::decay_t<Ex>> (Util::AsTypelist_t<typename Result::L_t> {}))
						return Result::Left (ex);
					else
						return Result::Left (std::current_exception ());
				}

				++RecLevel_;

				try
				{
					return F_ ();
				}
				catch (const vmime::exceptions::authentication_error& err)
				{
					const auto& respStr = QString::fromUtf8 (err.response ().c_str ());

					qWarning () << "Snails::detail::ExceptionsHandler::operator():"
							<< "caught auth error:"
							<< respStr;

					return Result::Left (err);
				}
				catch (const vmime::exceptions::connection_error& e)
				{
					return (*this) (e);
				}
				catch (const vmime::exceptions::command_error& e)
				{
					return (*this) (e);
				}
				catch (const vmime::exceptions::invalid_response& e)
				{
					return (*this) (e);
				}
				catch (const vmime::exceptions::operation_timed_out& e)
				{
					return (*this) (e);
				}
				catch (const vmime::exceptions::not_connected& e)
				{
					ReconnectATW (W_);

					return (*this) (e);
				}
				catch (const vmime::exceptions::socket_exception& e)
				{
					return (*this) (e);
				}
				catch (const vmime::security::cert::certificateException& e)
				{
					return Result::Left (e);
				}
				catch (const std::exception&)
				{
					return Result::Left (std::current_exception ());
				}
			}
		};

		template<typename Result, typename F>
		Result HandleExceptions (AccountThreadWorker *w, F&& f)
		{
			return ExceptionsHandler<Result, F> { w, std::forward<F> (f), detail::MaxRecLevel } ();
		}

		template<typename Right>
		struct WrapFunctionTypeImpl
		{
			using Result_t = Util::Either<InvokeError_t<>, Right>;

			template<typename F>
			static auto WrapFunction (AccountThreadWorker *w, const F& f)
			{
				return [=] (auto... args)
				{
					return HandleExceptions<Result_t> (w,
							[&]
							{
								return Result_t::Right (std::invoke (f, args...));
							});
				};
			}
		};

		template<>
		struct WrapFunctionTypeImpl<void>
		{
			using Result_t = Util::Either<InvokeError_t<>, Util::Void>;

			template<typename F>
			static auto WrapFunction (AccountThreadWorker *w, const F& f)
			{
				return [=] (auto... args)
				{
					return HandleExceptions<Result_t> (w,
							[&]
							{
								std::invoke (f, args...);
								return Result_t::Right ({});
							});
				};
			}
		};

		template<typename... Lefts, typename Right>
		struct WrapFunctionTypeImpl<Util::Either<std::variant<Lefts...>, Right>>
		{
			using Result_t = Util::Either<InvokeError_t<Lefts...>, Right>;

			template<typename F>
			static auto WrapFunction (AccountThreadWorker *w, const F& f)
			{
				return [=] (auto... args)
				{
					return HandleExceptions<Result_t> (w, [&] { return Result_t::LeftLift (std::invoke (f, args...)); });
				};
			}
		};

		template<typename... Args, typename F>
		auto WrapFunction (AccountThreadWorker *w, const F& f)
		{
			return WrapFunctionTypeImpl<std::result_of_t<F (AccountThreadWorker*, Args...)>>::WrapFunction (w, f);
		}
	}

	template<typename T>
	using WrapReturnType_t = typename detail::WrapFunctionTypeImpl<T>::Result_t;

	template<typename F, typename... Args>
	using WrapFunctionType_t = WrapReturnType_t<std::result_of_t<F (AccountThreadWorker*, Args...)>>;

	class AccountThread : public Util::WorkerThread<AccountThreadWorker>
	{
		std::atomic_bool IsRunning_;
	public:
		using WorkerThread::WorkerThread;

		template<typename F, typename... Args>
		auto Schedule (TaskPriority prio, const F& func, const Args&... args)
		{
			QFutureInterface<WrapFunctionType_t<F, Args...>> iface;
			return Schedule (iface, prio, func, args...);
		}

		template<typename F, typename... Args>
		auto Schedule (QFutureInterface<WrapFunctionType_t<F, Args...>> iface,
				TaskPriority, const F& func, const Args&... args)
		{
			auto reporting = [this, func, iface, args...] (AccountThreadWorker *w) mutable
			{
				IsRunning_ = true;

				iface.reportStarted ();
				Util::ReportFutureResult (iface, detail::WrapFunction<Args...> (w, func), w, args...);

				IsRunning_ = false;
			};

			ScheduleImpl (std::move (reporting));

			return iface.future ();
		}

		size_t GetQueueSize () override;
	};
}
}

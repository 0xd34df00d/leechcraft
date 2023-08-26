/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <coroutine>
#include <stdexcept>
#include <QMetaObject>
#include <QVector>

namespace LC::Util
{
	namespace detail
	{
		struct DeadObjectInfo
		{
			std::string ClassName_;
			QString ObjectName_;
		};

		auto MakeDeadObjectMessage (const DeadObjectInfo& info)
		{
			const std::string prefix = "coroutine's context object " + info.ClassName_;
			if (info.ObjectName_.isEmpty ())
				return prefix + " died";
			else
				return prefix + " (" + info.ObjectName_.toStdString () + ") died";
		}
	}

	class ContextDeadException : public std::runtime_error
	{
	public:
		explicit ContextDeadException (const detail::DeadObjectInfo& info)
		: std::runtime_error { detail::MakeDeadObjectMessage (info) }
		{
		}
	};

	namespace detail
	{
		template<typename T>
		auto Awaiter (T& obj)
		{
			if constexpr (requires { operator co_await (obj); })
				return operator co_await (obj);
			else if constexpr (requires { obj.operator co_await (); })
				return obj.operator co_await ();
			else
				return obj;
		}

		template<typename Promise, typename T>
		struct AwaitableWrapper
		{
			Promise& Promise_;
			T Orig_;

			bool await_ready ()
			{
				return Awaiter (Orig_).await_ready ();
			}

			decltype (auto) await_suspend (auto handle)
			{
				return Awaiter (Orig_).await_suspend (handle);
			}

			decltype (auto) await_resume ()
			{
				if (!Promise_.DeadObjects_.isEmpty ())
					throw ContextDeadException { Promise_.DeadObjects_.front () };
				return Awaiter (Orig_).await_resume ();
			}
		};
	}

	template<typename>
	struct ContextExtensions
	{
		using HasContextExtensions = void;

		QVector<QMetaObject::Connection> ContextConnections_;
		QVector<detail::DeadObjectInfo> DeadObjects_;

		void FinalSuspend () noexcept
		{
			for (auto conn : ContextConnections_)
				QObject::disconnect (conn);
		}

		template<typename T>
		auto await_transform (T&& awaitable)
		{
			return detail::AwaitableWrapper<ContextExtensions, T> { *this, std::forward<T> (awaitable) };
		}
	};

	struct RegisterContext
	{
		QObject& Context_;

		explicit RegisterContext (QObject& context)
		: Context_ { context }
		{
		}

		bool await_ready () const noexcept
		{
			return false;
		}

		template<typename Promise>
			requires requires { typename Promise::HasContextExtensions; }
		bool await_suspend (std::coroutine_handle<Promise> handle)
		{
			auto conn = QObject::connect (&Context_,
					&QObject::destroyed,
					[handle] (QObject *object)
					{
						auto className = object->metaObject ()->className ();
						handle.promise ().DeadObjects_.push_back ({ className, object->objectName () });
					});
			handle.promise ().ContextConnections_.push_back (conn);
			return false;
		}

		void await_resume ()
		{
		}
	};
}

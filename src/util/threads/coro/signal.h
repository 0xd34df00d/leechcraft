/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <concepts>
#include <coroutine>
#include <tuple>
#include <type_traits>
#include <utility>
#include <QObject>
#include <util/sll/typelist.h>

namespace LC::Util::detail
{
	template<typename T>
	concept PrivateSignalTag = std::same_as<T, class T::QPrivateSignal>;

	template<std::derived_from<QObject> Obj, typename... Args>
	class SignalAwaiter
	{
		template<template<typename...> typename Cont, typename... Brgs>
		struct StripPrivate { using Type = Cont<std::decay_t<Brgs>...>; };

		template<template<typename...> typename Cont, typename... Brgs>
			requires (sizeof... (Brgs) > 0) && PrivateSignalTag<Brgs... [sizeof... (Brgs) - 1]>
		struct StripPrivate<Cont, Brgs...>
		{
			using Type = decltype (Init (Cont<std::decay_t<Brgs>...> {}));
		};

		template<typename... Brgs>
		struct RetHolder : QObject
		{
			std::optional<std::tuple<Brgs...>> Result_ {};
			std::coroutine_handle<> Cont_ {};

			void Handle (Brgs... brgs)
			{
				Result_.emplace (std::tuple { std::move (brgs)... });
				QMetaObject::invokeMethod (this, Cont_, Qt::QueuedConnection);
			}

			void HandleDestroyed ()
			{
				if (!Result_)
					QMetaObject::invokeMethod (this, Cont_, Qt::QueuedConnection);
			}

			auto GetRet ()
			{
				constexpr auto BrgsCount = sizeof... (Brgs);

				if constexpr (BrgsCount == 1)
					return std::move (Result_)
							.transform ([] (std::tuple<Brgs...>&& tup) { return std::get<0> (std::move (tup)); });
				else if constexpr (BrgsCount > 1)
					return std::move (Result_);
				else
					return static_cast<bool> (Result_);
			}
		};

		using Holder = StripPrivate<RetHolder, Args...>::Type;
	public:
		Obj& Obj_;
		void (Obj::*Sig_) (Args...);
		Holder Holder_ {};

		bool await_ready () const noexcept
		{
			return false;
		}

		void await_suspend (std::coroutine_handle<> handle) noexcept
		{
			Holder_.Cont_ = handle;
			QObject::connect (&Obj_,
					Sig_,
					&Holder_,
					&Holder::Handle,
					Qt::SingleShotConnection);
			QObject::connect (&Obj_,
					&QObject::destroyed,
					&Holder_,
					&Holder::HandleDestroyed);
		}

		auto await_resume ()
		{
			return Holder_.GetRet ();
		}
	};
}

namespace LC::Util
{
	template<std::derived_from<QObject> Obj, typename... Args>
	struct Signal
	{
		Obj& Obj_;
		void (Obj::*Sig_) (Args...);

		detail::SignalAwaiter<Obj, Args...> operator co_await () const
		{
			return { Obj_, Sig_ };
		}
	};

	template<std::derived_from<QObject> Base, std::derived_from<Base> Derived, typename... Args>
	Signal (Derived&, void (Base::*) (Args...)) -> Signal<Base, Args...>;
}

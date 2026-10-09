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
#include <exception>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <QObject>
#include <util/sll/typelist.h>
#include <util/sll/void.h>

namespace LC::Util::detail
{
	template<typename T>
	concept PrivateSignalTag = std::same_as<T, class T::QPrivateSignal>;

	template<typename... Args>
	struct TuplePacker
	{
		using Ret = std::tuple<std::decay_t<Args>...>;
		using Arg = Void;

		Arg Arg_ {};

		Ret Collect (Args... args) const
		{
			return Ret { args... };
		}

		static auto GetRet (std::optional<Ret>&& result)
		{
			constexpr auto ArgsCount = sizeof... (Args);

			if constexpr (ArgsCount == 1)
				return std::move (result)
						.transform ([] (Ret&& tup) { return std::get<0> (std::move (tup)); });
			else if constexpr (ArgsCount > 1)
				return std::move (result);
			else
				return static_cast<bool> (result);
		}
	};

	template<typename FunType>
	struct FunHandler
	{
		template<typename... Args>
		struct Handler
		{
			using Ret = std::invoke_result_t<FunType, Args...>;
			using Arg = FunType;

			Arg Fun_ {};

			Ret Collect (Args... args)
			{
				return std::invoke (Fun_, std::move (args)...);
			}

			static auto GetRet (std::optional<Ret>&& result)
			{
				return std::move (result);
			}
		};
	};

	template<
			std::derived_from<QObject> Obj,
			template<typename...> typename SyncHandler,
			typename Sig,
			typename... SlotArgs
		>
	struct SignalAwaiter
	{
		struct RetHolder : QObject
		{
			using Handler = SyncHandler<SlotArgs...>;
			Handler Handler_;

			std::optional<typename Handler::Ret> Result_ {};
			std::coroutine_handle<> Cont_ {};
			std::exception_ptr Exception_ {};

			explicit(false) RetHolder (Handler::Arg arg)
			: Handler_ { arg }
			{
			}

			void Handle (SlotArgs... args)
			{
				try
				{
					Result_.emplace (Handler_.Collect (std::move (args)...));
				}
				catch (...)
				{
					Exception_ = std::current_exception ();
				}

				QMetaObject::invokeMethod (this, Cont_, Qt::QueuedConnection);
			}

			void HandleDestroyed ()
			{
				if (!Result_ && !Exception_)
					QMetaObject::invokeMethod (this, Cont_, Qt::QueuedConnection);
			}

			auto GetRet ()
			{
				if (Exception_)
					std::rethrow_exception (Exception_);
				return Handler::GetRet (std::move (Result_));
			}
		};

		Obj& Obj_;
		Sig Sig_;
		RetHolder Holder_;

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
					&RetHolder::Handle,
					Qt::SingleShotConnection);
			QObject::connect (&Obj_,
					&QObject::destroyed,
					&Holder_,
					&RetHolder::HandleDestroyed);
		}

		auto await_resume ()
		{
			return Holder_.GetRet ();
		}
	};

	template<
			std::derived_from<QObject> Obj,
			template<typename...> typename SyncHandler,
			typename Sig,
			typename... SlotArgs
		>
	struct SignalBase
	{
		Obj& Obj_;
		Sig Sig_;

		SyncHandler<SlotArgs...>::Arg SyncHandlerArg_ {};

		SignalAwaiter<Obj, SyncHandler, Sig, SlotArgs...> operator co_await () const
		{
			return { Obj_, Sig_, { SyncHandlerArg_ } };
		}
	};
}

namespace LC::Util
{
	template<std::derived_from<QObject> Obj, template<typename...> typename SyncHandler, typename Sig, typename SlotArgsList>
	struct Signal {};

	template<std::derived_from<QObject> Obj, template<typename...> typename SyncHandler, typename Sig, typename... SlotArgs>
	struct Signal<Obj, SyncHandler, Sig, Typelist<SlotArgs...>> : detail::SignalBase<Obj, SyncHandler, Sig, SlotArgs...> {};

	template<std::derived_from<QObject> Obj, template<typename...> typename SyncHandler, typename Sig, typename... SlotArgs>
		requires (sizeof... (SlotArgs) > 0) && detail::PrivateSignalTag<SlotArgs... [sizeof... (SlotArgs) - 1]>
	struct Signal<Obj, SyncHandler, Sig, Typelist<SlotArgs...>> : Signal<Obj, SyncHandler, Sig, decltype (Init (Typelist<SlotArgs...> {}))> {};

	template<
			std::derived_from<QObject> Base,
			std::derived_from<Base> Derived,
			typename... SignalArgs
		>
	Signal (Derived&, void (Base::*) (SignalArgs...))
		-> Signal<
					Base,
					detail::TuplePacker,
					void (Base::*) (SignalArgs...),
					Typelist<SignalArgs...>
				>;

	template<
			std::derived_from<QObject> Base,
			std::derived_from<Base> Derived,
			typename Handler,
			typename... SignalArgs
		>
	Signal (Derived&, void (Base::*) (SignalArgs...), Handler)
		-> Signal<
					Base,
					detail::FunHandler<std::decay_t<Handler>>::template Handler,
					void (Base::*) (SignalArgs...),
					Typelist<SignalArgs...>
				>;
}

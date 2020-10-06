/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <tuple>
#include <memory>
#include <functional>
#include <any>
#include <QReadWriteLock>
#include <QtDebug>

using Tox = struct Tox;

namespace LC::Azoth::Sarin
{
	namespace detail
	{
		template<typename First, typename R, typename... Args>
		std::tuple<Args...> DetectArgsImpl (R (First::*) (Args...));

		template<typename First, typename R, typename... Args>
		std::tuple<Args...> DetectArgsImpl (R (*) (First*, Args...));

		template<typename First, typename F>
		auto DetectArgsImpl (F&& f) -> decltype (DetectArgsImpl<First> (+f));

		template<typename First, typename T>
		using DetectArgs = decltype (DetectArgsImpl<First> (std::declval<T> ()));

		struct RegHolder
		{
			struct Base
			{
				virtual ~Base () = default;

				virtual bool IsEqual (const Base&) const = 0;

				virtual size_t Hash () const = 0;
			};

			std::shared_ptr<Base> Base_;

			template<typename RegType>
			struct Holder final : Base
			{
				const RegType Reg_;

				constexpr Holder (RegType reg)
				: Reg_ { reg }
				{
				}

				bool IsEqual (const Base& otherBase) const override
				{
					const auto other = dynamic_cast<const Holder<RegType>*> (&otherBase);
					if (!other)
						return false;

					return other->Reg_ == Reg_;
				}

				size_t Hash () const override
				{
					return std::hash<RegType> {} (Reg_);
				}
			};

			template<typename RegType>
			RegHolder (RegType reg)
			: Base_ { std::make_shared<Holder<RegType>> (reg) }
			{
			}

			friend bool operator== (const RegHolder& left, const RegHolder& right)
			{
				return left.Base_->IsEqual (*right.Base_);
			}

			friend size_t qHash (const RegHolder& holder)
			{
				return holder.Base_->Hash ();
			}
		};

		struct CBHolder
		{
			std::any F_;

			template<typename This, typename F, typename Tuple>
			struct Maker;

			template<typename This, typename F, typename... Args>
			struct Maker<This, F, std::tuple<Args...>>
			{
				CBHolder operator() (This pThis, F cb) const
				{
					return
					{
						std::function<void (Args...)> { [pThis, cb] (Args... args) { std::invoke (cb, pThis, args...); } }
					};
				}
			};

			template<typename... Args>
			void Invoke (Args... args) const
			{
				const auto res = std::any_cast<std::function<void (Args...)>> (&F_);
				if (!res)
					qWarning () << Q_FUNC_INFO
							<< "mismatched parameters, expected typeid:"
							<< F_.type ().name ();
				else
					(*res) (args...);
			}
		};

		template<typename T>
		class Locked
		{
			mutable QReadWriteLock Lock_;
			T Ent_;
		public:
			Locked () = default;

			Locked (Locked&& other)
			: Ent_ { other.WithWrite ([] (auto& ent) -> decltype (auto) { return std::move (ent); }) }
			{
			}

			Locked (const Locked& other)
			: Ent_ { other.WithRead ([] (const auto& ent) -> decltype (auto) { return ent; }) }
			{
			}

			T& UnsafeGet ()
			{
				return Ent_;
			};

			const T& UnsafeGet () const
			{
				return Ent_;
			}

			template<typename F>
			decltype (auto) WithRead (F&& f) const
			{
				QReadLocker locker { &Lock_ };
				return std::invoke (f, Ent_);
			}

			template<typename F>
			decltype (auto) WithWrite (F&& f)
			{
				QWriteLocker locker { &Lock_ };
				return std::invoke (f, Ent_);
			}
		};
	}

	class CallbackManager
	{
		detail::Locked<std::weak_ptr<Tox>> Tox_;

		template<auto Reg, typename>
		struct MakeWrapper;

		using LockedCbList_t = detail::Locked<QList<detail::CBHolder>>;
		using Callbacks_t = detail::Locked<QHash<detail::RegHolder, LockedCbList_t>>;
		Callbacks_t Callbacks_;

		detail::Locked<QList<std::function<void (Tox*)>>> ProxyReggers_;
	public:
		void SetTox (const std::shared_ptr<Tox>&);

		template<auto Reg, typename F, typename This>
		void Register (This pThis, F callback)
		{
			using Args = detail::DetectArgs<std::remove_pointer_t<This>, F>;

			auto& cbList = Callbacks_.WithWrite ([] (auto& cbs) -> decltype (auto) { return cbs [detail::RegHolder { Reg }]; });

			const bool shouldReg = cbList.WithWrite ([&] (auto& list)
					{
						list.push_back (detail::CBHolder::Maker<This, F, Args> {} (pThis, callback));
						return list.size () == 1;
					});

			if (shouldReg)
			{
				auto proxyRegger = [] (Tox *tox) { Reg (tox, MakeWrapper<Reg, Args> {} ()); };
				ProxyReggers_.WithWrite ([&] (auto& regs) { regs << proxyRegger; });

				if (const auto tox = Tox_.WithRead ([] (const auto& tox) { return tox.lock (); }))
					proxyRegger (tox.get ());
			}
		}
	private:
		template<auto Reg, typename... Args>
		void InvokeCallbacks (Args... args)
		{
			const auto& cbList = Callbacks_.WithRead ([] (const auto& cbs) -> decltype (auto) { return cbs [detail::RegHolder { Reg }]; });
			cbList.WithRead ([&] (const auto& list)
					{
						for (const auto& item : list)
							item.Invoke (args...);
					});
		}
	};

	template<auto Reg, typename... Args>
	struct CallbackManager::MakeWrapper<Reg, std::tuple<Args...>>
	{
		auto operator() () const
		{
			return [] (Tox*, Args... args, void *udata)
			{
				auto cbMgr = static_cast<CallbackManager*> (udata);
				cbMgr->InvokeCallbacks<Reg> (args...);
			};
		}
	};
}

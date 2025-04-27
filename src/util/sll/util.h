/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <stdexcept>

namespace LC
{
namespace Util
{
	class [[nodiscard]] SharedScopeGuard
	{
		std::shared_ptr<void> Guard_;
		public:
		template<typename F>
		SharedScopeGuard (const F& f)
		: Guard_ { nullptr, [f] (void*) { f (); } }
		{
		}

		SharedScopeGuard () = delete;

		SharedScopeGuard (const SharedScopeGuard&) = default;
		SharedScopeGuard (SharedScopeGuard&&) = default;

		SharedScopeGuard& operator= (const SharedScopeGuard&) = default;
		SharedScopeGuard& operator= (SharedScopeGuard&&) = default;
	};

	namespace detail
	{
		using DefaultScopeGuardDeleter = std::function<void ()>;

		template<typename F>
		class [[nodiscard]] ScopeGuard
		{
			F F_;
			bool Perform_ = true;
		public:
			ScopeGuard () noexcept
			: F_ {}
			, Perform_ { false }
			{
			}

			ScopeGuard (const F& f) noexcept
			: F_ { f }
			{
			}

			template<typename F1, typename F2>
				requires std::is_same_v<F, DefaultScopeGuardDeleter>
			explicit ScopeGuard (ScopeGuard<F1>&& g1, ScopeGuard<F2>&& g2)
			: F_
			{
				[f1 = std::move (g1.F_), p1 = g1.Perform_, f2 = std::move (g2.F_), p2 = g2.Perform_]
				{
					if (p1)
						f1 ();
					if (p2)
						f2 ();
				}
			}
			, Perform_ { g1.Perform_ || g2.Perform_ }
			{
				g1.Perform_ = false;
				g2.Perform_ = false;
			}

			ScopeGuard (const ScopeGuard&) = delete;
			ScopeGuard& operator= (const ScopeGuard&) = delete;

			ScopeGuard& operator= (ScopeGuard&& other)
			{
				if (Perform_)
					F_ ();

				F_ = other.F_;
				Perform_ = other.Perform_;
				other.Perform_ = false;
				return *this;
			}

			ScopeGuard (ScopeGuard&& other) noexcept
			: F_ { other.F_ }
			, Perform_ { other.Perform_ }
			{
				other.Perform_ = false;
			}

			~ScopeGuard ()
			{
				if (Perform_)
					F_ ();
			}

			void Dismiss () noexcept
			{
				Perform_ = false;
			}

			ScopeGuard<DefaultScopeGuardDeleter> EraseType ()
			{
				Dismiss ();
				return ScopeGuard<DefaultScopeGuardDeleter> { F_ };
			}

			operator ScopeGuard<DefaultScopeGuardDeleter> ()
			{
				return EraseType ();
			}

			SharedScopeGuard Shared () &&
			{
				if (!Perform_)
					throw std::logic_error { "this scope guard has already been converted to a shared one" };

				Perform_ = false;
				return { F_ };
			}
		};
	}

	using DefaultScopeGuard = detail::ScopeGuard<detail::DefaultScopeGuardDeleter>;

	/** @brief Returns an object performing passed function on scope exit.
	 *
	 * The returned object performs the passed function \em f upon
	 * destruction (and, thus, on scope exit).
	 *
	 * The object is not copyable and not movable, and otherwise is
	 * implementation-defined.
	 *
	 * Typical usage:
	 * \code{.cpp}
		QSettings settings { "OrgNameName", "AppName" };
		settings.beginGroup ();
		const auto guard = Util::MakeScopeGuard ([&settings] { settings.endGroup (); });
		// ...
	   \endcode
	 *
	 * @param[in] f The function to execute on scope exit. Should be a
	 * callable without any arguments.
	 * @return An object executing \em f on destruction.
	 */
	template<typename F>
	[[nodiscard]] detail::ScopeGuard<F> MakeScopeGuard (const F& f)
	{
		return { f };
	}
}
}

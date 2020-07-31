/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>

namespace LC
{
namespace Util
{
	/** @brief A peir of two functions, typically a continuation and an
	 * error handler.
	 *
	 * Inspired by Haskell's Data.Either, the functions contained in
	 * EitherCont are called \em left and \em right respectively.
	 *
	 * The common convention is to store the "normal" continuation in the
	 * \em right function, while the \em left one stores the error handler.
	 *
	 * @tparam LeftSig The signature of the \em left function.
	 * @tparam RightSig The signature of the \em right function.
	 */
	template<typename LeftSig, typename RightSig>
	class EitherCont
	{
		using Left_f = std::function<LeftSig>;
		using Right_f = std::function<RightSig>;
		Left_f Left_;
		Right_f Right_;
	public:
		/** @brief Default-constructs the continuation with uninitialized
		 * functions.
		 */
		EitherCont () = default;

		/** @brief Sets the left and right functions to \em l and \em r.
		 *
		 * @tparam L The type of the left functor which should be used to
		 * initialize the left function (must be convertible to
		 * \em LeftSig).
		 * @tparam R The type of the right functor which should be used to
		 * initialize the left function (must be convertible to
		 * \em LeftSig).
		 */
		template<typename L, typename R>
		EitherCont (const L& l, const R& r)
		: Left_ { l }
		, Right_ { r }
		{
		}

		/** @brief Checks if both functions are initialized.
		 *
		 * Calling any function on an object for which this function
		 * returns false leads to undefined behavior.
		 *
		 * @return Whether both function are initialized.
		 */
		explicit operator bool () const
		{
			return Left_ && Right_;
		}

		/** @brief Invoke the left function and return its result.
		 *
		 * Invokes the left function forwarding \em args of type \em Args
		 * (which may be different from the types in \em LeftSig, but
		 * convertible to them), and returns its result.
		 *
		 * @param[in] args The arguments to forward to the left function.
		 * @return The result of invoking the left function with \em args.
		 * @tparam Args The types of arguments to pass to the left
		 * function.
		 *
		 * @sa Right()
		 */
		template<typename... Args>
		auto Left (Args&&... args) const
		{
			return Left_ (std::forward<Args> (args)...);
		}

		/** @brief Invoke the right function and return its result.
		 *
		 * Invokes the right function forwarding \em args of type \em Args
		 * (which may be different from the types in \em RightSig, but
		 * convertible to them), and returns its result.
		 *
		 * @param[in] args The arguments to forward to the right function.
		 * @return The result of invoking the right function with \em args.
		 * @tparam Args The types of arguments to pass to the right
		 * function.
		 *
		 * @sa Left()
		 */
		template<typename... Args>
		auto Right (Args&&... args) const
		{
			return Right_ (std::forward<Args> (args)...);
		}
	};
}
}

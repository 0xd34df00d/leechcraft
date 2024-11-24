/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <optional>
#include "typeclassutil.h"
#include "void.h"
#include "either.h"

namespace LC
{
namespace Util
{
	/** @brief The Functor class is used for types that can be mapped over.
	 *
	 * Minimal complete definition:
	 * - Apply() function and FmapResult_t alias.
	 *
	 * For a reference imolementation please see InstanceFunctor<std::optional<T>>.
	 *
	 * @tparam T The functor type instantiated with some concrete
	 * containee type.
	 *
	 * @sa Fmap()
	 * @sa IsFunctor()
	 */
	template<typename T>
	struct InstanceFunctor
	{
		using UndefinedTag = void;

		/** @brief The type of the functor after its elements were
		 * mapped by the function \em F.
		 *
		 * This type should correspond to the return type of the Apply()
		 * function when passed this functor and a function of type
		 * \em F.
		 *
		 * @tparam F The type of the function to apply to the elements
		 * inside this functor.
		 */
		template<typename F>
		using FmapResult_t = detail::ImplementationType;

		/** @brief Applies the \em function to the each of the elements
		 * inside the \em functor.
		 *
		 * @param[in] functor The functor whose values are subject to
		 * \em function.
		 * @param[in] function The function that should be applied to the
		 * values in the \em functor.
		 * @return A functor of type FmapResult_t<F> where each element
		 * the result of applying the \em function to the corresponding element
		 * in the source \em functor.
		 *
		 * @tparam F The type of the \em function to apply to the elements
		 * in the function.
		 */
		template<typename F>
		static FmapResult_t<F> Apply (const T& functor, const F& function);
	};

	namespace detail
	{
		template<typename T>
		constexpr bool IsFunctorImpl (int, typename InstanceFunctor<T>::UndefinedTag* = nullptr)
		{
			return false;
		}

		template<typename T>
		constexpr bool IsFunctorImpl (float)
		{
			return true;
		}
	}

	/** @brief Checks whether the given type has a Functor instance for it.
	 *
	 * @return Whether type T implements the Functor class.
	 *
	 * @tparam T The type to check.
	 */
	template<typename T>
	constexpr bool IsFunctor ()
	{
		return detail::IsFunctorImpl<T> (0);
	}

	/** @brief The result type of the contents of the functor \em T mapped
	 * by function \em F.
	 *
	 * @tparam T The type of the functor.
	 * @tparam F The type of the function to apply to the elements inside the
	 * functor.
	 */
	template<typename T, typename F>
	using FmapResult_t = typename InstanceFunctor<T>::template FmapResult_t<F>;

	/** @brief Apply the function \em f to the elements in \em functor.
	 *
	 * This function forwards the function \em f to the instance of the
	 * Functor class (namely, InstanceFunctor<T>) for the type \em T to do
	 * the actual function application.
	 *
	 * @param[in] functor The functor whose values are subject to
	 * \em function.
	 * @param[in] function The function that should be applied to the
	 * values in the functor.
	 * @return A functor of type FmapResult_t<T, F> where each element is
	 * the result of applying the \em function to the corresponding element
	 * in the source \em functor.
	 *
	 * @tparam T The type of the functor.
	 * @tparam F The type of the function to apply to the elements inside
	 * the functor.
	 *
	 * @sa InstanceFunctor
	 */
	template<typename T, typename F, typename = std::enable_if_t<IsFunctor<T> ()>>
	FmapResult_t<T, F> Fmap (const T& functor, const F& function)
	{
		return InstanceFunctor<T>::Apply (functor, function);
	}

	/** @brief An operator-style alias for Fmap().
	 *
	 * This operator allows writing Fmap()'s in infix form. Internally, it
	 * just forwards the call to Fmap().
	 *
	 * @param[in] functor The functor whose values are subject to
	 * \em function.
	 * @param[in] function The function that should be applied to the
	 * values in the functor.
	 * @return A functor of type FmapResult_t<T, F> where each element is
	 * the result of applying the \em function to the corresponding element
	 * in the source \em functor.
	 *
	 * @tparam T The type of the functor.
	 * @tparam F The type of the function to apply to the elements inside
	 * the functor.
	 *
	 * @sa InstanceFunctor
	 * @sa Fmap()
	 */
	template<typename T, typename F>
	auto operator* (const F& function, const T& functor) -> decltype (Fmap (functor, function))
	{
		return Fmap (functor, function);
	}

	/** @brief An operator-style alias for Fmap().
	 *
	 * This operator allows writing Fmap()'s in infix form. Internally, it
	 * just forwards the call to Fmap().
	 *
	 * @param[in] functor The functor whose values are subject to
	 * \em function.
	 * @param[in] function The function that should be applied to the
	 * values in the functor.
	 * @return A functor of type FmapResult_t<T, F> where each element is
	 * the result of applying the \em function to the corresponding element
	 * in the source \em functor.
	 *
	 * @tparam T The type of the functor.
	 * @tparam F The type of the function to apply to the elements inside
	 * the functor.
	 *
	 * @sa InstanceFunctor
	 * @sa Fmap()
	 */
	template<typename T, typename F>
	auto operator* (const T& functor, const F& function) -> decltype (Fmap (functor, function))
	{
		return Fmap (functor, function);
	}

	namespace detail
	{
		template<typename T>
		struct WrapVoidResult
		{
			using Type = T;
		};

		template<>
		struct WrapVoidResult<void>
		{
			using Type = Void;
		};

		template<typename T>
		using WrapVoidResult_t = typename WrapVoidResult<T>::Type;
	}

	/** @brief Implementation of the Functor class for std::optional.
	 *
	 * The implementation applies the function to the contents of the
	 * std::optional if it's not empty, otherwise it just leaves an
	 * empty std::optional.
	 *
	 * This is analogous to the Maybe type.
	 *
	 * @tparam T The element type contained inside the std::optional.
	 */
	template<typename T>
	struct InstanceFunctor<std::optional<T>>
	{
		template<typename F>
		using FmapResult_t = std::optional<detail::WrapVoidResult_t<std::decay_t<std::invoke_result_t<F, T>>>>;

		template<typename F>
		static FmapResult_t<F> Apply (const std::optional<T>& t, const F& f)
		{
			if (!t)
				return {};

			if constexpr (std::is_same_v<FmapResult_t<F>, std::optional<Void>>)
			{
				std::invoke (f, *t);
				return { Void {} };
			}
			else
				return { std::invoke (f, *t) };
		}
	};

	template<typename L, typename R>
	struct InstanceFunctor<Either<L, R>>
	{
		template<typename F>
		using FmapResult_t = Either<L, std::invoke_result_t<F, R>>;

		template<typename F>
		static FmapResult_t<F> Apply (const Either<L, R>& either, const F& f)
		{
			if (either.IsLeft ())
				return FmapResult_t<F>::Left (either.GetLeft ());

			return FmapResult_t<F>::Right (f (either.GetRight ()));
		}
	};
}
}

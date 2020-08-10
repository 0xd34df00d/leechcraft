/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
namespace Util
{
	/** @brief A somewhat "strong" typedef.
	 *
	 * \em NewType provides Haskell's newtype-like semantics.
	 *
	 * Typical usage is as follows:
	 *	\code
		using FirstAlias = Util::NewType<BaseType, struct FirstTag>;
		using SecondAlias = Util::NewType<BaseType, struct SecondTag>;
		\endcode
	 *
	 * After this, <code>FirstAlias</code> and <code>SecondAlias</code>
	 * become more or less independent from C++'s type system point of
	 * view.
	 *
	 * @tparam T The type for which to create the params for.
	 */
	template<typename T, typename Tag>
	class NewType : public T
	{
	public:
		using T::T;

		NewType () = default;

		NewType (const T& t)
		: T { t }
		{
		}
	};
}
}

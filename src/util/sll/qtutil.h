/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sllconfig.h"
#include <QLatin1String>

namespace LC::Util
{
	namespace detail
	{
		template<template<typename, typename> class PairType, typename BaseIt, typename Key, typename Value>
		struct Iterator
		{
			using iterator_category = std::bidirectional_iterator_tag;
			using difference_type = std::ptrdiff_t;

			using value_type = PairType<Key, Value>;
			using reference = value_type;
			using pointer = value_type*;

			BaseIt BaseIt_;

			bool operator== (const Iterator& other) const { return BaseIt_ == other.BaseIt_; }
			bool operator!= (const Iterator& other) const { return BaseIt_ != other.BaseIt_; }

			reference operator* () { return { BaseIt_.key (), BaseIt_.value () }; }

			Iterator& operator++ ()
			{
				++BaseIt_;
				return *this;
			}

			Iterator operator++ (int) & { return { BaseIt_++ }; }

			Iterator& operator-- ()
			{
				--BaseIt_;
				return *this;
			}

			Iterator operator-- (int) & { return { BaseIt_-- }; }
		};

		template<typename Iter, typename BaseIter, typename Assoc>
		struct Range
		{
			Assoc Assoc_;

			BaseIter Begin_ = Assoc_.begin ();
			BaseIter End_ = Assoc_.end ();

			auto begin () const { return Iter { Begin_ }; }
			auto end () const { return Iter { End_ }; }
		};
	}

	/** @brief Converts an Qt's associative sequence \em assoc to an
	 * STL-like iteratable range.
	 *
	 * This function takes an associative container \em assoc (one of
	 * Qt's containers like QHash and QMap) and returns a range with
	 * <code>value_type</code> equal to <code>PairType<K, V></code>.
	 *
	 * This way, both the key and the value of each pair in the \em assoc
	 * can be accessed in a range-for loop, for example.
	 *
	 * Example usage:
	 *	\code
		QMap<QString, int> someMap;
		for (const auto& pair : Util::Stlize (someMap))
			qDebug () << pair.first		// outputs a QString key
					<< pair.second;		// outputs an integer value corresponding to the key
		\endcode
	 *
	 * All kinds of accesses are supported: elements of a non-const
	 * container may be modified via the iterators in the returned range.
	 *
	 * @param[in] assoc The Qt's associative container to iterate over.
	 * @return A range with iterators providing access to both the key
	 * and the value via its <code>value_type</code>.
	 *
	 * @tparam PairType The type of the pairs that should be used in the
	 * resulting range's iterators' <code>value_type</code>.
	 * @tparam Assoc The type of the source Qt associative container.
	 */
	template<template<typename K, typename V> class PairType = std::pair, typename Assoc>
	auto Stlize (Assoc&& assoc)
	{
		using BaseIt = decltype (assoc.begin ());
		using Iterator = detail::Iterator<
				PairType,
				BaseIt,
				decltype (BaseIt {}.key ()),
				decltype (BaseIt {}.value ())
			>;
		return detail::Range<Iterator, BaseIt, Assoc> { std::forward<Assoc> (assoc) };
	}
}

namespace LC
{
	inline QLatin1String operator"" _ql (const char *str, std::size_t size)
	{
		return QLatin1String { str, static_cast<int> (size) };
	}
}

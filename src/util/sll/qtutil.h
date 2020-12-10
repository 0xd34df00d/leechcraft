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
	/** @brief Converts an Qt's associative sequence \em assoc to an
	 * STL-like iteratable range.
	 *
	 * This function takes an associative container \em assoc (one of
	 * Qt's containers like QHash and QMap) and returns a range with
	 * <code>value_type</code> equal to <code>std::pair<K, V></code>.
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
	 * @tparam Assoc The type of the source Qt associative container.
	 */
	template<typename Assoc>
	auto Stlize (Assoc&& assoc)
	{
		struct Range
		{
			Assoc Assoc_;

			auto begin () const { return Assoc_.keyValueBegin (); }
			auto end () const { return Assoc_.keyValueEnd (); }
		};

		return Range { std::forward<Assoc> (assoc) };
	}

	inline QByteArray AsByteArray (std::string_view view)
	{
		return QByteArray::fromRawData (view.data (), view.size ());
	}

	inline std::string_view AsStringView (const QByteArray& arr)
	{
		return { arr.constData (), static_cast<size_t> (arr.size ()) };
	}
}

namespace LC
{
	inline QLatin1String operator"" _ql (const char *str, std::size_t size)
	{
		return QLatin1String { str, static_cast<int> (size) };
	}
}

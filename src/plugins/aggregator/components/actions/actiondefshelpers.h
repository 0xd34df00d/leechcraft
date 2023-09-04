/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple.hpp>

#define MAKE_CASE(_, index, tuple) \
	case BOOST_PP_TUPLE_ELEM(index, tuple): \
		return "Action" BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(index, tuple)) "_";

#define MAKE_ACTIONS(Context, ...) \
	enum class Context::ActionId { __VA_ARGS__ }; \
	namespace						\
	{								\
		constexpr auto AllActionIds () { using enum Context::ActionId; return std::array { __VA_ARGS__ }; } \
		QByteArray ToString (Context::ActionId action) \
		{							\
			using enum Context::ActionId;	\
			switch (action) 		\
			{						\
			BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE((__VA_ARGS__)), MAKE_CASE, (__VA_ARGS__)) \
			}						\
			qWarning () << "unknown action" << static_cast<int> (action);	\
			return {};														\
		}	\
	}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "common.h"

using LC::operator""_ct;

struct SimpleRecord
{
	lco::PKey<int, lco::NoAutogen> ID_;
	QString Value_;

	constexpr static auto ClassName = "SimpleRecord"_ct;

	auto AsTuple () const
	{
		return std::tie (ID_, Value_);
	}
};

BOOST_FUSION_ADAPT_STRUCT (SimpleRecord,
		ID_,
		Value_)

TOSTRING (SimpleRecord)

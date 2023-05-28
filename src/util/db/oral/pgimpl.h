/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/visitor.h>
#include "oraltypes.h"

namespace LC::Util::oral::detail::PostgreSQL
{
	struct ImplFactory
	{
		using IsImpl_t = void;

		struct TypeLits
		{
			inline constexpr static CtString IntAutoincrement { "SERIAL PRIMARY KEY" };
			inline constexpr static CtString Binary { "BYTEA" };
		};

		inline constexpr static CtString LimitNone { "ALL" };

		constexpr static auto GetInsertPrefix (auto)
		{
			return "INSERT"_ct;
		}

		constexpr static auto GetInsertSuffix (InsertAction::DefaultTag)
		{
			return ""_ct;
		}

		constexpr static auto GetInsertSuffix (InsertAction::IgnoreTag)
		{
			return "ON CONFLICT DO NOTHING"_ct;
		}

		constexpr static auto GetInsertSuffix (InsertAction::Replace, auto conflictingFields, auto allFields)
		{
			return "ON CONFLICT (" + JoinTup (conflictingFields, ", ") +
					") DO UPDATE SET " + JoinTup (ZipWith (allFields, " = EXCLUDED.", allFields), ", ");
		}
	};
}

namespace LC::Util::oral
{
	using PostgreSQLImplFactory = detail::PostgreSQL::ImplFactory;
}

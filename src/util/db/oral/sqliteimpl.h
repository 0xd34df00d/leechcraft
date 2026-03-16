/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "oraltypes.h"

namespace LC::Util::oral::detail::SQLite
{
	struct ImplFactory
	{
		using IsImpl_t = void;

		struct TypeLits
		{
			constexpr static CtString IntAutoincrement { "INTEGER PRIMARY KEY AUTOINCREMENT" };
			constexpr static CtString Binary { "BLOB" };
		};

		constexpr static CtString LimitNone { "-1" };

		constexpr static auto GetInsertPrefix (InsertAction::IgnoreTag)
		{
			return "INSERT OR IGNORE"_ct;
		}

		constexpr static auto GetInsertPrefix (InsertAction::DefaultTag)
		{
			return "INSERT"_ct;
		}

		template<auto... Ptrs>
		constexpr static auto GetInsertPrefix (InsertAction::Replace::FieldsType<Ptrs...>)
		{
			return "INSERT"_ct;
		}

		constexpr static auto GetInsertPrefix (InsertAction::Replace::WholeType)
		{
			return "INSERT"_ct;
		}

		constexpr static auto GetInsertSuffix (InsertAction::DefaultTag)
		{
			return ""_ct;
		}

		constexpr static auto GetInsertSuffix (InsertAction::IgnoreTag)
		{
			return ""_ct;
		}

		constexpr static auto GetInsertSuffix (InsertAction::Replace, auto fields)
		{
			return "ON CONFLICT DO UPDATE SET " + JoinTup (ZipWith (fields, " = EXCLUDED.", fields), ", ");
		}

		constexpr static auto GetInsertSuffix (InsertAction::DefaultTag, auto pkName)
		{
			return "RETURNING "_ct + pkName;
		}

		constexpr static auto GetInsertSuffix (InsertAction::IgnoreTag, auto pkName)
		{
			return "RETURNING "_ct + pkName;
		}

		constexpr static auto GetInsertSuffix (InsertAction::Replace, auto fields, auto pkName)
		{
			return "ON CONFLICT DO UPDATE SET " + JoinTup (ZipWith (fields, " = EXCLUDED.", fields), ", ") +
					" RETURNING " + pkName;
		}
	};
}

namespace LC::Util::oral
{
	using SQLiteImplFactory = detail::SQLite::ImplFactory;
}

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
			inline constexpr static CtString IntAutoincrement { "INTEGER PRIMARY KEY AUTOINCREMENT" };
			inline constexpr static CtString Binary { "BLOB" };
		};

		inline constexpr static CtString LimitNone { "-1" };

		constexpr static auto GetInsertPrefix (InsertAction::DefaultTag)
		{
			return "INSERT"_ct;
		}

		constexpr static auto GetInsertPrefix (InsertAction::IgnoreTag)
		{
			return "INSERT OR IGNORE"_ct;
		}

		constexpr static auto GetInsertPrefix (InsertAction::Replace::PKeyType)
		{
			return "INSERT OR REPLACE"_ct;
		}

		template<auto... Ptrs>
		constexpr static auto GetInsertPrefix (InsertAction::Replace::FieldsType<Ptrs...>)
		{
			return "INSERT OR REPLACE"_ct;
		}

		constexpr static auto GetInsertSuffix (auto...)
		{
			return ""_ct;
		}
	};
}

namespace LC::Util::oral
{
	using SQLiteImplFactory = detail::SQLite::ImplFactory;
}

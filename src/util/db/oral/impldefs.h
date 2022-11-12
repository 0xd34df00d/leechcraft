/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "oraldetailfwd.h"
#include "oraltypes.h"

class QSqlQuery;

namespace LC::Util::oral::detail
{
	class IInsertQueryBuilder
	{
	public:
		virtual ~IInsertQueryBuilder () = default;

		virtual QSqlQuery GetQuery (InsertAction) = 0;
	};

	using IInsertQueryBuilder_ptr = std::unique_ptr<IInsertQueryBuilder>;
}

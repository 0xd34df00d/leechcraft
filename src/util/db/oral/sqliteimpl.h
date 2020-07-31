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
#include "oraldetailfwd.h"
#include "impldefs.h"

namespace LC::Util::oral::detail::SQLite
{
	using QSqlQuery_ptr = std::shared_ptr<QSqlQuery>;

	class InsertQueryBuilder final : public IInsertQueryBuilder
	{
		const QSqlDatabase DB_;

		std::array<QSqlQuery_ptr, InsertAction::StaticCount () + 1> Queries_;
		const QString InsertSuffix_;
	public:
		InsertQueryBuilder (const QSqlDatabase& db, const CachedFieldsData& data)
		: DB_ { db }
		, InsertSuffix_ { " INTO " + data.Table_ +
			" (" + data.Fields_.join (", ") + ") VALUES (" +
			data.BoundFields_.join (", ") + ");" }
		{
		}

		QSqlQuery_ptr GetQuery (InsertAction action) override
		{
			auto& query = Queries_ [action.Selector_.index ()];
			if (!query)
			{
				query = std::make_shared<QSqlQuery> (DB_);
				query->prepare (GetInsertPrefix (action) + InsertSuffix_);
			}
			return query;
		}
	private:
		QString GetInsertPrefix (InsertAction action)
		{
			return Visit (action.Selector_,
					[] (InsertAction::DefaultTag) { return "INSERT"; },
					[] (InsertAction::IgnoreTag) { return "INSERT OR IGNORE"; },
					[] (InsertAction::Replace) { return "INSERT OR REPLACE"; });
		}
	};

	class ImplFactory
	{
	public:
		struct TypeLits
		{
			inline static const QString IntAutoincrement { "INTEGER PRIMARY KEY AUTOINCREMENT" };
			inline static const QString Binary { "BLOB" };
		};

		inline static const QString LimitNone { "-1" };

		auto MakeInsertQueryBuilder (const QSqlDatabase& db, const CachedFieldsData& data) const
		{
			return std::make_unique<InsertQueryBuilder> (db, data);
		}
	};
}

namespace LC::Util::oral
{
	using SQLiteImplFactory = detail::SQLite::ImplFactory;
}

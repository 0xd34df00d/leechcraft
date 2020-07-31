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

namespace LC::Util::oral::detail::PostgreSQL
{
	using QSqlQuery_ptr = std::shared_ptr<QSqlQuery>;

	class InsertQueryBuilder final : public IInsertQueryBuilder
	{
		const QSqlDatabase DB_;

		QSqlQuery_ptr Default_;
		QSqlQuery_ptr Ignore_;

		const QString InsertBase_;
		const QString Updater_;
	public:
		InsertQueryBuilder (const QSqlDatabase& db, const CachedFieldsData& data)
		: DB_ { db }
		, InsertBase_ { "INSERT INTO " + data.Table_ +
				" (" + data.Fields_.join (", ") + ") VALUES (" +
				data.BoundFields_.join (", ") + ") " }
		, Updater_ { Map (data.Fields_, [] (auto&& str) { return str + " = EXCLUDED." + str; }).join (", ") }
		{
		}

		QSqlQuery_ptr GetQuery (InsertAction action) override
		{
			return Visit (action.Selector_,
					[this] (InsertAction::DefaultTag) { return GetDefaultQuery (); },
					[this] (InsertAction::IgnoreTag) { return GetIgnoreQuery (); },
					[this] (InsertAction::Replace ct) { return MakeReplaceQuery (ct.Fields_); });
		}
	private:
		QSqlQuery_ptr GetDefaultQuery ()
		{
			if (!Default_)
			{
				Default_ = std::make_shared<QSqlQuery> (DB_);
				Default_->prepare (InsertBase_);
			}
			return Default_;
		}

		QSqlQuery_ptr GetIgnoreQuery ()
		{
			if (!Default_)
			{
				Default_ = std::make_shared<QSqlQuery> (DB_);
				Default_->prepare (InsertBase_ + "ON CONFLICT DO NOTHING");
			}
			return Default_;
		}

		QSqlQuery_ptr MakeReplaceQuery (const QStringList& constraining)
		{
			auto query = std::make_shared<QSqlQuery> (DB_);
			query->prepare (InsertBase_ + GetReplacer (constraining));
			return query;
		}

		QString GetReplacer (const QStringList& constraining) const
		{
			return "ON CONFLICT (" +
					constraining.join (", ") +
					") DO UPDATE SET " +
					Updater_;
		}
	};

	class ImplFactory
	{
	public:
		struct TypeLits
		{
			inline static const QString IntAutoincrement { "SERIAL PRIMARY KEY" };
			inline static const QString Binary { "BYTEA" };
		};

		inline static const QString LimitNone { "ALL" };

		auto MakeInsertQueryBuilder (const QSqlDatabase& db, const CachedFieldsData& data) const
		{
			return std::make_unique<InsertQueryBuilder> (db, data);
		}
	};
}

namespace LC::Util::oral
{
	using PostgreSQLImplFactory = detail::PostgreSQL::ImplFactory;
}

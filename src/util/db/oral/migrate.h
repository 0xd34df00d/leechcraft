/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "util/db/oral/oral.h"

namespace LC::Util::oral
{
	namespace detail
	{
		bool MatchesSchema (const QString& baseName, const QString& schema, QSqlDatabase& db)
		{
			auto result = Util::RunTextQuery (db,
					"SELECT sql FROM sqlite_master WHERE type = 'table' AND name = '%1'"_qs
							.arg (baseName));
			if (!result.next ())
				return true;

			const auto& existingDDL = result.value (0).toString ();

			auto figureOutFields = [] (const QString& str)
			{
				auto firstOpen = str.indexOf ('(');
				auto lastClose = str.lastIndexOf (')');
				return str.midRef (firstOpen, lastClose - firstOpen);
			};
			auto existing = figureOutFields (existingDDL);
			auto suggested = figureOutFields (schema);
			return existing == suggested;
		}
	}

	template<typename Record, typename ImplFactory = SQLiteImplFactory>
	void Migrate (QSqlDatabase& db)
	{
		constexpr auto baseName = Record::ClassName ();
		constexpr auto thisName = "copy" + baseName;
		const auto& schema = detail::AdaptCreateTableNamed<thisName, ImplFactory, Record> ().ToString ();

		const auto& baseNameStr = baseName.ToString ();
		const auto& thisNameStr = thisName.ToString ();
		if (detail::MatchesSchema (baseNameStr, schema, db))
		{
			qDebug () << Q_FUNC_INFO
					<< "not migrating"
					<< db.connectionName ();
			return;
		}

		qDebug () << Q_FUNC_INFO
				<< "migrating"
				<< db.connectionName ();

		Util::DBLock lock { db };
		lock.Init ();

		Util::RunTextQuery (db, schema);

		constexpr auto fields = JoinTup (detail::FieldNames<Record>, ", ");

		Util::RunTextQuery (db,
				"INSERT INTO %2 (%1) SELECT %1 FROM %3;"_qs
						.arg (fields.ToString (), thisNameStr, baseNameStr));

		Util::RunTextQuery (db,
				"DROP TABLE %1;"_qs
						.arg (baseNameStr));
		Util::RunTextQuery (db,
				"ALTER TABLE %1 RENAME TO %2;"_qs
						.arg (thisNameStr, baseNameStr));

		lock.Good ();
	}
}

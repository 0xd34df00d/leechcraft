/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsstoragetest.h"
#include <algorithm>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>
#include <util/db/util.h>
#include <util/sll/qtutil.h>
#include "tagsstorage.h"

QTEST_GUILESS_MAIN (LC::TagsStorageTest)

namespace LC
{
	namespace
	{
		const QUuid A { "{00000000-0000-0000-0000-00000000000a}"_qs };
		const QUuid B { "{00000000-0000-0000-0000-00000000000b}"_qs };
		const QUuid C { "{00000000-0000-0000-0000-00000000000c}"_qs };

		using Rows = QList<std::pair<QUuid, QString>>;

		Rows Sorted (Rows rows)
		{
			std::ranges::sort (rows);
			return rows;
		}

		QStringList Sorted (QStringList list)
		{
			list.sort ();
			return list;
		}

		auto WithConnection (const QString& path, auto&& f)
		{
			const auto& name = Util::GenConnectionName ("org.LeechCraft.Core.TagsStorageTest"_qs);
			auto result = [&]
			{
				auto db = QSqlDatabase::addDatabase ("QSQLITE"_qs, name);
				db.setDatabaseName (path);
				db.open ();
				return f (db);
			} ();
			QSqlDatabase::removeDatabase (name);
			return result;
		}

		QStringList Tables (const QString& path)
		{
			return WithConnection (path, [] (const QSqlDatabase& db) { return Sorted (db.tables ()); });
		}

		QString OtherPayload (const QString& path)
		{
			return WithConnection (path,
					[] (const QSqlDatabase& db)
					{
						QSqlQuery query { db };
						query.exec ("SELECT Payload FROM Other"_qs);
						return query.next () ? query.value (0).toString () : QString {};
					});
		}

		bool SeedLegacyTables (const QString& path, const Rows& rows)
		{
			return WithConnection (path,
					[&rows] (const QSqlDatabase& db)
					{
						QSqlQuery query { db };
						if (!query.exec ("CREATE TABLE Tags (Id BLOB NOT NULL UNIQUE, Name TEXT NOT NULL)"_qs) ||
								!query.exec ("CREATE TABLE Other (Payload TEXT)"_qs) ||
								!query.exec ("INSERT INTO Other VALUES ('kept')"_qs) ||
								!query.prepare ("INSERT INTO Tags (Id, Name) VALUES (?, ?)"_qs))
							return false;

						for (const auto& [id, name] : rows)
						{
							query.addBindValue (id.toByteArray ());
							query.addBindValue (name);
							if (!query.exec ())
								return false;
						}
						return true;
					});
		}
	}

	void TagsStorageTest::testIdHoldsSeveralNames ()
	{
		QTemporaryDir dir;
		const auto& path = dir.filePath ("core.db"_qs);

		TagsStorage storage { path };
		QCOMPARE (Tables (path), QStringList { "TagNames"_qs });

		storage.AddTag (A, "x"_qs);
		storage.AddTag (A, "y"_qs);
		storage.AddTag (B, "x"_qs);
		storage.AddTag (A, "x"_qs);
		QCOMPARE (Sorted (storage.GetAllTags ()), Sorted ({ { A, "x"_qs }, { A, "y"_qs }, { B, "x"_qs } }));
	}

	void TagsStorageTest::testReplaceName ()
	{
		TagsStorage storage { ":memory:"_qs };
		storage.AddTag (A, "x"_qs);
		storage.AddTag (B, "x"_qs);
		storage.AddTag (B, "y"_qs);

		storage.ReplaceName ("x"_qs, { "p"_qs, "q"_qs });
		QCOMPARE (Sorted (storage.GetAllTags ()),
				Sorted ({ { A, "p"_qs }, { A, "q"_qs }, { B, "p"_qs }, { B, "q"_qs }, { B, "y"_qs } }));

		storage.ReplaceName ("q"_qs, { "y"_qs });
		QCOMPARE (Sorted (storage.GetAllTags ()),
				Sorted ({ { A, "p"_qs }, { A, "y"_qs }, { B, "p"_qs }, { B, "y"_qs } }));

		storage.ReplaceName ("y"_qs, { "y"_qs, "z"_qs });
		QCOMPARE (Sorted (storage.GetAllTags ()),
				Sorted ({ { A, "p"_qs }, { A, "y"_qs }, { A, "z"_qs }, { B, "p"_qs }, { B, "y"_qs }, { B, "z"_qs } }));

		storage.ReplaceName ("p"_qs, {});
		storage.ReplaceName ("nonexistent"_qs, { "w"_qs });
		QCOMPARE (Sorted (storage.GetAllTags ()),
				Sorted ({ { A, "y"_qs }, { A, "z"_qs }, { B, "y"_qs }, { B, "z"_qs } }));
	}

	void TagsStorageTest::testUnparsableIdsAreSkipped ()
	{
		QTemporaryDir dir;
		const auto& path = dir.filePath ("core.db"_qs);
		{
			TagsStorage storage { path };
			storage.AddTag (A, "kept"_qs);
		}
		QVERIFY (WithConnection (path,
				[] (const QSqlDatabase& db)
				{
					QSqlQuery query { db };
					return query.exec ("INSERT INTO TagNames (Id, Name) VALUES ('garbage', 'lost')"_qs);
				}));

		TagsStorage reopened { path };
		QCOMPARE (reopened.GetAllTags (), (Rows { { A, "kept"_qs } }));
	}

	void TagsStorageTest::testLegacyTableIsMigrated ()
	{
		QTemporaryDir dir;
		const auto& path = dir.filePath ("core.db"_qs);
		const Rows legacy { { A, "work"_qs }, { B, "work"_qs }, { C, " x;y "_qs } };
		QVERIFY (SeedLegacyTables (path, legacy));

		{
			TagsStorage storage { path };
			QCOMPARE (Sorted (storage.GetAllTags ()), Sorted (legacy));
		}
		QCOMPARE (Tables (path), (QStringList { "Other"_qs, "TagNames"_qs }));
		QCOMPARE (OtherPayload (path), "kept"_qs);

		TagsStorage reopened { path };
		QCOMPARE (Sorted (reopened.GetAllTags ()), Sorted (legacy));
		QCOMPARE (Tables (path), (QStringList { "Other"_qs, "TagNames"_qs }));
	}
}

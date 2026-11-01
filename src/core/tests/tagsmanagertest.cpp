/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsmanagertest.h"
#include <memory>
#include <QDataStream>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTest>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/tags/emitters/tagsmanager.h>
#include "tagsmanager.h"
#include "tagsstorage.h"

QTEST_GUILESS_MAIN (LC::TagsManagerTest)

namespace LC
{
	namespace
	{
		const QUuid Low { "{00000000-0000-0000-0000-000000000001}"_qs };
		const QUuid High { "{00000000-0000-0000-0000-000000000002}"_qs };

		const auto Work = "work"_qs;

		const QRegularExpression InvalidNameWarning { "^invalid tag name "_qs };

		using Rows = QList<std::pair<QUuid, QString>>;

		std::unique_ptr<TagsStorage> Seeded (const Rows& rows)
		{
			auto storage = std::make_unique<TagsStorage> (":memory:"_qs);
			for (const auto& [id, name] : rows)
				storage->AddTag (id, name);
			return storage;
		}

		struct Fixture
		{
			TagsStorage& Storage_;
			TagsManager Manager_;

			explicit Fixture (const Rows& rows = {})
			: Fixture { Seeded (rows) }
			{
			}
		private:
			explicit Fixture (std::unique_ptr<TagsStorage> storage)
			: Storage_ { *storage }
			, Manager_ { std::move (storage) }
			{
			}
		};

		QStringList Sorted (QStringList list)
		{
			list.sort ();
			return list;
		}

		QStringList StoredNames (const TagsStorage& storage)
		{
			QStringList result;
			for (const auto& [id, name] : storage.GetAllTags ())
				result << name;
			return Sorted (result);
		}

		QString Describe (const TagsManager::InvalidTagName& error)
		{
			return Util::Visit (error,
					[] (TagsManager::BlankTagName) { return "blank"_qs; },
					[] (const TagsManager::DelimiterInTagName& e) { return "delimiter " + e.Delimiter_; });
		}

		bool MakeReadOnly (const QStringList& otherConnections)
		{
			for (const auto& name : QSqlDatabase::connectionNames ())
				if (!otherConnections.contains (name))
					return QSqlQuery { QSqlDatabase::database (name) }.exec ("PRAGMA query_only = 1"_qs);
			return false;
		}

		void IgnoreQueryFailure (const QString& kind)
		{
			QTest::ignoreMessage (QtCriticalMsg, QRegularExpression { '^' + kind + " query execution failed$"_qs });
			QTest::ignoreMessage (QtCriticalMsg, QRegularExpression { "^query: "_qs });
			QTest::ignoreMessage (QtCriticalMsg, QRegularExpression { "."_qs });
			QTest::ignoreMessage (QtCriticalMsg, QRegularExpression { "^bound keys: "_qs });
			QTest::ignoreMessage (QtCriticalMsg, QRegularExpression { "^bound values: "_qs });
		}
	}

	void TagsManagerTest::testIdsAreBracedUuidStrings ()
	{
		const auto idString = "{12345678-1234-5678-9abc-123456789abc}"_qs;

		Fixture f { Rows { { QUuid { idString }, Work } } };

		const auto& id = f.Manager_.GetID (Work);
		QVERIFY (id);
		QCOMPARE (id->Id_, idString);
		QCOMPARE (f.Manager_.GetTags (TagId { idString }), QStringList { Work });
		QVERIFY (f.Manager_.GetTags (TagId { Low.toString () }).isEmpty ());
		QVERIFY (f.Manager_.GetTags (TagId { "not an id"_qs }).isEmpty ());
	}

	void TagsManagerTest::testGetIdCreatesAndPersists ()
	{
		Fixture f;
		QSignalSpy replaced { &f.Manager_.GetEmitter (), &Util::Emitters::TagsManager::nameReplaced };
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };

		const auto& id = f.Manager_.GetID (Work);
		QVERIFY (id);
		QCOMPARE (f.Manager_.GetTags (*id), QStringList { Work });
		QCOMPARE (f.Manager_.rowCount ({}), 1);
		QCOMPARE (inserted.size (), 1);
		QCOMPARE (replaced.size (), 0);
		QVERIFY (f.Storage_.GetAllTags ().contains (std::pair { QUuid { id->Id_ }, Work }));

		QVERIFY (f.Manager_.GetID (Work) == id);
		QCOMPARE (f.Manager_.rowCount ({}), 1);
		QCOMPARE (inserted.size (), 1);
	}

	void TagsManagerTest::testGetIdTrims ()
	{
		Fixture f;

		const auto& id = f.Manager_.GetID (" work "_qs);
		QVERIFY (id);
		QCOMPARE (f.Manager_.GetTags (*id), QStringList { Work });
		QVERIFY (f.Manager_.GetID (Work) == id);
		QCOMPARE (f.Manager_.rowCount ({}), 1);
		QCOMPARE (StoredNames (f.Storage_), QStringList { Work });
	}

	void TagsManagerTest::testGetIdRefusesInvalidNames_data ()
	{
		QTest::addColumn<QString> ("name");

		QTest::newRow ("empty") << QString {};
		QTest::newRow ("blank") << "   "_qs;
		QTest::newRow ("delimiter") << "a;b"_qs;
		QTest::newRow ("delimiter only") << ";"_qs;
	}

	void TagsManagerTest::testGetIdRefusesInvalidNames ()
	{
		QFETCH (QString, name);

		Fixture f;
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };

		QTest::ignoreMessage (QtWarningMsg, InvalidNameWarning);
		QVERIFY (!f.Manager_.GetID (name));
		QCOMPARE (f.Manager_.rowCount ({}), 0);
		QCOMPARE (inserted.size (), 0);
		QVERIFY (StoredNames (f.Storage_).isEmpty ());
	}

	void TagsManagerTest::testGetIdsSkipsRefusedNames ()
	{
		Fixture f;

		QTest::ignoreMessage (QtWarningMsg, InvalidNameWarning);
		QTest::ignoreMessage (QtWarningMsg, InvalidNameWarning);
		const auto& ids = f.Manager_.GetIDs ({ "a"_qs, " "_qs, "b;c"_qs, "d"_qs });
		QCOMPARE (ids.size (), 2);
		QCOMPARE (f.Manager_.GetTags (ids), (QStringList { "a"_qs, "d"_qs }));
		QVERIFY (f.Manager_.SplitToIDs ("a; d"_qs) == ids);
	}

	void TagsManagerTest::testDuplicateNamesResolveToTheLowestId ()
	{
		const auto dup = "dup"_qs;

		Fixture f { Rows { { High, dup }, { Low, dup } } };
		QCOMPARE (f.Manager_.rowCount ({}), 1);

		const auto& id = f.Manager_.GetID (dup);
		QVERIFY (id);
		QCOMPARE (id->Id_, Low.toString ());
		QCOMPARE (f.Manager_.rowCount ({}), 1);
	}

	void TagsManagerTest::testGetIdIsPure ()
	{
		Fixture f;
		const auto& impure = f.Manager_.GetID ("a"_qs);
		QVERIFY (impure);
		QVERIFY (f.Manager_.ReplaceName ("a"_qs, { "a"_qs, "b"_qs }).IsRight ());
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };

		const auto& pure = f.Manager_.GetID ("a"_qs);
		QVERIFY (pure);
		QVERIFY (pure != impure);
		QCOMPARE (f.Manager_.GetTags (*pure), QStringList { "a"_qs });
		QVERIFY (f.Manager_.GetID ("a"_qs) == pure);
		QCOMPARE (inserted.size (), 0);
		QCOMPARE (f.Manager_.GetAllTags (), (QStringList { "a"_qs, "b"_qs }));
		QCOMPARE (StoredNames (f.Storage_), (QStringList { "a"_qs, "a"_qs, "b"_qs }));
	}

	void TagsManagerTest::testGetTagsDedupes ()
	{
		const auto dup = "dup"_qs;

		Fixture f { Rows { { High, dup }, { Low, dup } } };
		const auto& a = f.Manager_.GetID ("a"_qs);
		QVERIFY (a);

		const QList<TagId> ids { TagId { High.toString () }, *a, TagId { Low.toString () }, *a };
		QCOMPARE (f.Manager_.GetTags (ids), (QStringList { dup, "a"_qs }));
		QCOMPARE (f.Manager_.JoinIDs (ids), "dup; a"_qs);
	}

	void TagsManagerTest::testStoredNamesAreRepaired_data ()
	{
		QTest::addColumn<QString> ("stored");
		QTest::addColumn<QString> ("repaired");

		QTest::newRow ("delimiter") << "x;y"_qs << "x y"_qs;
		QTest::newRow ("surrounding whitespace") << "  a  "_qs << "a"_qs;
		QTest::newRow ("both") << " x;y "_qs << "x y"_qs;
	}

	void TagsManagerTest::testStoredNamesAreRepaired ()
	{
		QFETCH (QString, stored);
		QFETCH (QString, repaired);

		Fixture f { Rows { { Low, stored } } };

		QCOMPARE (f.Manager_.GetTags (TagId { Low.toString () }), QStringList { repaired });
		QCOMPARE (f.Manager_.GetAllTags (), QStringList { repaired });
		QCOMPARE (StoredNames (f.Storage_), QStringList { repaired });
	}

	void TagsManagerTest::testRepairMergesIntoAnExistingName ()
	{
		Fixture f { Rows { { High, "p;q"_qs }, { Low, "p q"_qs } } };

		QCOMPARE (f.Manager_.GetAllTags (), QStringList { "p q"_qs });
		QCOMPARE (f.Manager_.GetTags (TagId { High.toString () }), QStringList { "p q"_qs });
		QCOMPARE (StoredNames (f.Storage_), (QStringList { "p q"_qs, "p q"_qs }));
		QVERIFY (f.Manager_.GetID ("p q"_qs) == TagId { Low.toString () });
	}

	void TagsManagerTest::testBlankAfterRepairIsKept ()
	{
		QTest::ignoreMessage (QtWarningMsg, "cannot repair tag \";\" to \"\"");
		Fixture f { Rows { { Low, ";"_qs } } };

		QCOMPARE (f.Manager_.GetAllTags (), QStringList { ";"_qs });
		QCOMPARE (StoredNames (f.Storage_), QStringList { ";"_qs });
	}

	void TagsManagerTest::testReplaceNameRefusals_data ()
	{
		QTest::addColumn<QString> ("newName");
		QTest::addColumn<QString> ("error");

		QTest::newRow ("empty") << QString {} << "blank"_qs;
		QTest::newRow ("blank") << "   "_qs << "blank"_qs;
		QTest::newRow ("delimiter") << "a;b"_qs << "delimiter ;"_qs;
	}

	void TagsManagerTest::testReplaceNameRefusals ()
	{
		QFETCH (QString, newName);
		QFETCH (QString, error);

		Fixture f;
		f.Manager_.GetID (Work);
		QSignalSpy replaced { &f.Manager_.GetEmitter (), &Util::Emitters::TagsManager::nameReplaced };
		QSignalSpy removed { &f.Manager_, &QAbstractItemModel::rowsRemoved };

		const auto& result = f.Manager_.ReplaceName (Work, { "fine"_qs, newName });
		QVERIFY (result.IsLeft ());
		QCOMPARE (Describe (result.GetLeft ()), error);
		QCOMPARE (f.Manager_.GetAllTags (), QStringList { Work });
		QCOMPARE (StoredNames (f.Storage_), QStringList { Work });
		QCOMPARE (replaced.size (), 0);
		QCOMPARE (removed.size (), 0);
	}

	void TagsManagerTest::testReplaceNameWithoutChange ()
	{
		Fixture f;
		f.Manager_.GetID (Work);
		QSignalSpy replaced { &f.Manager_.GetEmitter (), &Util::Emitters::TagsManager::nameReplaced };

		QVERIFY (f.Manager_.ReplaceName (Work, { " work "_qs }).IsRight ());
		QVERIFY (f.Manager_.ReplaceName ("nonexistent"_qs, { "whatever"_qs }).IsRight ());
		QVERIFY (f.Manager_.ReplaceName ("nonexistent"_qs, {}).IsRight ());
		QCOMPARE (replaced.size (), 0);
		QCOMPARE (f.Manager_.GetAllTags (), QStringList { Work });
		QCOMPARE (StoredNames (f.Storage_), QStringList { Work });
	}

	void TagsManagerTest::testRename ()
	{
		Fixture f;
		const auto& id = f.Manager_.GetID (Work);
		QVERIFY (id);
		QSignalSpy replaced { &f.Manager_.GetEmitter (), &Util::Emitters::TagsManager::nameReplaced };
		QSignalSpy removed { &f.Manager_, &QAbstractItemModel::rowsRemoved };
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };

		QVERIFY (f.Manager_.ReplaceName (Work, { " renamed "_qs }).IsRight ());
		QCOMPARE (f.Manager_.GetTags (*id), QStringList { "renamed"_qs });
		QVERIFY (f.Manager_.GetID ("renamed"_qs) == id);
		QCOMPARE (f.Manager_.GetAllTags (), QStringList { "renamed"_qs });
		QCOMPARE (removed.size (), 1);
		QCOMPARE (inserted.size (), 1);
		QCOMPARE (replaced.size (), 1);
		QCOMPARE (replaced.first (), (QVariantList { Work, QStringList { "renamed"_qs } }));
		QCOMPARE (StoredNames (f.Storage_), QStringList { "renamed"_qs });
	}

	void TagsManagerTest::testSplit ()
	{
		Fixture f;
		const auto& id = f.Manager_.GetID ("haskell hackage"_qs);
		QVERIFY (id);
		QSignalSpy replaced { &f.Manager_.GetEmitter (), &Util::Emitters::TagsManager::nameReplaced };
		QSignalSpy removed { &f.Manager_, &QAbstractItemModel::rowsRemoved };
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };

		QVERIFY (f.Manager_.ReplaceName ("haskell hackage"_qs, { "haskell"_qs, "hackage"_qs, "haskell"_qs }).IsRight ());
		QCOMPARE (f.Manager_.GetTags (*id), (QStringList { "hackage"_qs, "haskell"_qs }));
		QCOMPARE (f.Manager_.GetAllTags (), (QStringList { "hackage"_qs, "haskell"_qs }));
		QCOMPARE (removed.size (), 1);
		QCOMPARE (inserted.size (), 2);
		QCOMPARE (replaced.size (), 1);
		QCOMPARE (replaced.first ().at (0).toString (), "haskell hackage"_qs);
		QCOMPARE (Sorted (replaced.first ().at (1).toStringList ()), (QStringList { "hackage"_qs, "haskell"_qs }));
		QCOMPARE (StoredNames (f.Storage_), (QStringList { "hackage"_qs, "haskell"_qs }));

		const auto& half = f.Manager_.GetID ("haskell"_qs);
		QVERIFY (half);
		QVERIFY (half != id);
		QCOMPARE (f.Manager_.GetTags (*half), QStringList { "haskell"_qs });

		const auto& both = f.Manager_.GetIDs ({ "haskell"_qs, "hackage"_qs });
		QCOMPARE (both.size (), 2);
		QVERIFY (!both.contains (*id));
		QCOMPARE (f.Manager_.GetTags (both), (QStringList { "haskell"_qs, "hackage"_qs }));
		QCOMPARE (f.Manager_.GetTags (*id), (QStringList { "hackage"_qs, "haskell"_qs }));
	}

	void TagsManagerTest::testMerge ()
	{
		Fixture f { Rows { { High, "a"_qs }, { Low, "b"_qs } } };
		QSignalSpy replaced { &f.Manager_.GetEmitter (), &Util::Emitters::TagsManager::nameReplaced };
		QSignalSpy removed { &f.Manager_, &QAbstractItemModel::rowsRemoved };
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };

		QVERIFY (f.Manager_.ReplaceName ("b"_qs, { "a"_qs }).IsRight ());
		QCOMPARE (replaced.size (), 1);
		QCOMPARE (replaced.first (), (QVariantList { "b"_qs, QStringList { "a"_qs } }));
		QCOMPARE (f.Manager_.GetTags (TagId { Low.toString () }), QStringList { "a"_qs });
		QCOMPARE (f.Manager_.GetTags (TagId { High.toString () }), QStringList { "a"_qs });
		QCOMPARE (f.Manager_.GetAllTags (), QStringList { "a"_qs });
		QCOMPARE (f.Manager_.JoinIDs ({ TagId { High.toString () }, TagId { Low.toString () } }), "a"_qs);
		QVERIFY (f.Manager_.GetID ("a"_qs) == TagId { Low.toString () });
		QCOMPARE (removed.size (), 1);
		QCOMPARE (inserted.size (), 0);
		QCOMPARE (StoredNames (f.Storage_), (QStringList { "a"_qs, "a"_qs }));
	}

	void TagsManagerTest::testReplaceNameKeepingTheOldName ()
	{
		Fixture f;
		const auto& id = f.Manager_.GetID ("a"_qs);
		QVERIFY (id);
		QSignalSpy removed { &f.Manager_, &QAbstractItemModel::rowsRemoved };
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };

		QVERIFY (f.Manager_.ReplaceName ("a"_qs, { "a"_qs, "b"_qs }).IsRight ());
		QCOMPARE (f.Manager_.GetTags (*id), (QStringList { "a"_qs, "b"_qs }));
		QCOMPARE (removed.size (), 0);
		QCOMPARE (inserted.size (), 1);
		QCOMPARE (StoredNames (f.Storage_), (QStringList { "a"_qs, "b"_qs }));
	}

	void TagsManagerTest::testRemove ()
	{
		Fixture f;
		const auto& id = f.Manager_.GetID (Work);
		QVERIFY (id);
		QSignalSpy replaced { &f.Manager_.GetEmitter (), &Util::Emitters::TagsManager::nameReplaced };
		QSignalSpy removed { &f.Manager_, &QAbstractItemModel::rowsRemoved };

		QVERIFY (f.Manager_.ReplaceName (Work, {}).IsRight ());
		QCOMPARE (f.Manager_.rowCount ({}), 0);
		QVERIFY (f.Manager_.GetTags (*id).isEmpty ());
		QCOMPARE (removed.size (), 1);
		QCOMPARE (replaced.size (), 1);
		QCOMPARE (replaced.first (), (QVariantList { Work, QStringList {} }));
		QVERIFY (StoredNames (f.Storage_).isEmpty ());
	}

	void TagsManagerTest::testStorageFailureLeavesTheManagerUntouched ()
	{
		const auto& connections = QSqlDatabase::connectionNames ();
		Fixture f { Rows { { Low, Work } } };
		QVERIFY (MakeReadOnly (connections));

		QSignalSpy removed { &f.Manager_, &QAbstractItemModel::rowsRemoved };
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };
		QSignalSpy replaced { &f.Manager_.GetEmitter (), &Util::Emitters::TagsManager::nameReplaced };

		IgnoreQueryFailure ("insert"_qs);
		QVERIFY_THROWS_EXCEPTION (std::exception, f.Manager_.ReplaceName (Work, { "renamed"_qs }));
		IgnoreQueryFailure ("delete"_qs);
		QVERIFY_THROWS_EXCEPTION (std::exception, f.Manager_.ReplaceName (Work, {}));
		IgnoreQueryFailure ("insert"_qs);
		QVERIFY_THROWS_EXCEPTION (std::exception, f.Manager_.GetID ("fresh"_qs));

		QCOMPARE (f.Manager_.GetTags (TagId { Low.toString () }), QStringList { Work });
		QCOMPARE (f.Manager_.GetAllTags (), QStringList { Work });
		QCOMPARE (removed.size (), 0);
		QCOMPARE (inserted.size (), 0);
		QCOMPARE (replaced.size (), 0);
		QCOMPARE (StoredNames (f.Storage_), QStringList { Work });
	}

	void TagsManagerTest::testModelRowsFollowGetAllTags ()
	{
		Fixture f { Rows { { High, "a"_qs }, { Low, "b"_qs } } };
		const QAbstractItemModel& tm = f.Manager_;

		const auto& names = f.Manager_.GetAllTags ();
		QCOMPARE (tm.rowCount ({}), names.size ());
		QCOMPARE (tm.columnCount ({}), 1);
		for (int i = 0; i < names.size (); ++i)
		{
			const auto& idx = tm.index (i, 0, {});
			QCOMPARE (idx.data ().toString (), names [i]);
			QCOMPARE (idx.data (Qt::EditRole).toString (), names [i]);
			QVERIFY (!idx.parent ().isValid ());
			QCOMPARE (tm.rowCount (idx), 0);
			QVERIFY (!tm.index (0, 0, idx).isValid ());
		}
		QVERIFY (!tm.index (names.size (), 0, {}).isValid ());
		QVERIFY (!tm.index (0, 1, {}).isValid ());
	}

	void TagsManagerTest::testJoinAndSplit ()
	{
		Fixture f;
		QCOMPARE (f.Manager_.Split ("a; b;c "_qs), (QStringList { "a"_qs, "b"_qs, "c"_qs }));
		QCOMPARE (f.Manager_.Join ({ "a"_qs, "b"_qs }), "a; b"_qs);

		const auto& ids = f.Manager_.GetIDs ({ "a"_qs, "b"_qs });
		QCOMPARE (f.Manager_.JoinIDs ({ ids [0], TagId { Low.toString () }, ids [1] }), "a; b"_qs);
	}

	void TagsManagerTest::testTagIdListIsARegisteredMetaType ()
	{
		Fixture f;
		QVERIFY (QMetaType::fromName ("QList<LC::TagId>").isValid ());

		const QList<TagId> original { TagId { Low.toString () }, TagId { High.toString () } };
		QByteArray blob;
		QDataStream out { &blob, QIODevice::WriteOnly };
		out << QVariant::fromValue (original);
		QCOMPARE (out.status (), QDataStream::Ok);

		QDataStream in { blob };
		QVariant loaded;
		in >> loaded;
		QCOMPARE (in.status (), QDataStream::Ok);
		QVERIFY (loaded.value<QList<TagId>> () == original);
	}
}

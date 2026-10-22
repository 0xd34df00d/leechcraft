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
#include <QtTest>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include "tagsmanager.h"
#include "tagsstorage.h"

QTEST_GUILESS_MAIN (LC::TagsManagerTest)

namespace LC
{
	namespace
	{
		const QUuid Low { "{00000000-0000-0000-0000-000000000001}"_qs };
		const QUuid High { "{00000000-0000-0000-0000-000000000002}"_qs };

		const auto Untagged = "untagged"_qs;
		const auto Work = "work"_qs;

		const QRegularExpression InvalidNameWarning { "^invalid tag name "_qs };

		using Rows = QList<QPair<QUuid, QString>>;

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

		QModelIndex IndexOf (const TagsManager& tm, const QString& name)
		{
			for (int i = 0; i < tm.rowCount ({}); ++i)
				if (const auto& idx = tm.index (i, 0, {}); idx.data ().toString () == name)
					return idx;
			return {};
		}

		QString Describe (const TagsManager::RenameError& error)
		{
			return Util::Visit (error,
					[] (TagsManager::BlankTagName) { return "blank"_qs; },
					[] (const TagsManager::DelimiterInTagName& e) { return "delimiter " + e.Delimiter_; },
					[] (const TagsManager::TagNameTaken& e) { return "taken " + e.Name_; });
		}
	}

	void TagsManagerTest::testFreshStorageGetsUntagged ()
	{
		Fixture f;
		QCOMPARE (f.Manager_.rowCount ({}), 1);
		QCOMPARE (f.Manager_.GetAllTags (), QStringList { Untagged });
		QCOMPARE (StoredNames (f.Storage_), QStringList { Untagged });
	}

	void TagsManagerTest::testStoredUntaggedIsReused ()
	{
		Fixture f { Rows { { Low, Untagged } } };
		QCOMPARE (f.Manager_.rowCount ({}), 1);
		QCOMPARE (StoredNames (f.Storage_), QStringList { Untagged });

		const auto& id = f.Manager_.GetID (Untagged);
		QVERIFY (id);
		QCOMPARE (id->Id_, Low.toString ());
	}

	void TagsManagerTest::testIdsAreBracedUuidStrings ()
	{
		const auto idString = "{12345678-1234-5678-9abc-123456789abc}"_qs;

		Fixture f { Rows { { QUuid { idString }, Work } } };

		const auto& id = f.Manager_.GetID (Work);
		QVERIFY (id);
		QCOMPARE (id->Id_, idString);
		QCOMPARE (f.Manager_.GetTag (TagId { idString }).value_or (QString {}), Work);
		QVERIFY (!f.Manager_.GetTag (TagId { Low.toString () }));
		QVERIFY (!f.Manager_.GetTag (TagId { "not an id"_qs }));
	}

	void TagsManagerTest::testGetIdCreatesAndPersists ()
	{
		Fixture f;
		QSignalSpy updated { &f.Manager_, &TagsManager::tagsUpdated };
		QSignalSpy inserted { &f.Manager_, &QAbstractItemModel::rowsInserted };

		const auto& id = f.Manager_.GetID (Work);
		QVERIFY (id);
		QCOMPARE (f.Manager_.GetTag (*id).value_or (QString {}), Work);
		QCOMPARE (f.Manager_.rowCount ({}), 2);
		QCOMPARE (inserted.size (), 1);
		QCOMPARE (updated.size (), 1);
		QCOMPARE (updated.first ().first ().toStringList (), f.Manager_.GetAllTags ());
		QVERIFY (f.Storage_.GetAllTags ().contains (QPair { QUuid { id->Id_ }, Work }));

		QVERIFY (f.Manager_.GetID (Work) == id);
		QCOMPARE (f.Manager_.rowCount ({}), 2);
		QCOMPARE (updated.size (), 1);
	}

	void TagsManagerTest::testGetIdTrims ()
	{
		Fixture f;

		const auto& id = f.Manager_.GetID (" work "_qs);
		QVERIFY (id);
		QCOMPARE (f.Manager_.GetTag (*id).value_or (QString {}), Work);
		QVERIFY (f.Manager_.GetID (Work) == id);
		QCOMPARE (f.Manager_.rowCount ({}), 2);
		QCOMPARE (StoredNames (f.Storage_), (QStringList { Untagged, Work }));
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
		QSignalSpy updated { &f.Manager_, &TagsManager::tagsUpdated };

		QTest::ignoreMessage (QtWarningMsg, InvalidNameWarning);
		QVERIFY (!f.Manager_.GetID (name));
		QCOMPARE (f.Manager_.rowCount ({}), 1);
		QCOMPARE (updated.size (), 0);
		QCOMPARE (StoredNames (f.Storage_), QStringList { Untagged });
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
		QCOMPARE (f.Manager_.rowCount ({}), 3);

		QTest::ignoreMessage (QtWarningMsg, "2 tags named \"dup\"");
		const auto& id = f.Manager_.GetID (dup);
		QVERIFY (id);
		QCOMPARE (id->Id_, Low.toString ());
		QCOMPARE (f.Manager_.rowCount ({}), 3);
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

		QCOMPARE (f.Manager_.GetTag (TagId { Low.toString () }).value_or (QString {}), repaired);
		QCOMPARE (Sorted (f.Manager_.GetAllTags ()), Sorted ({ repaired, Untagged }));
		QCOMPARE (StoredNames (f.Storage_), Sorted ({ repaired, Untagged }));
	}

	void TagsManagerTest::testUnrepairableStoredNamesAreKept_data ()
	{
		QTest::addColumn<QStringList> ("stored");
		QTest::addColumn<QString> ("warning");

		QTest::newRow ("collision") << QStringList { "p;q"_qs, "p q"_qs } << "cannot repair tag \"p;q\" to \"p q\""_qs;
		QTest::newRow ("blank after repair") << QStringList { ";"_qs } << "cannot repair tag \";\" to \"\""_qs;
	}

	void TagsManagerTest::testUnrepairableStoredNamesAreKept ()
	{
		QFETCH (QStringList, stored);
		QFETCH (QString, warning);

		Rows rows;
		for (const auto& name : stored)
			rows << QPair { QUuid::createUuid (), name };

		QTest::ignoreMessage (QtWarningMsg, qPrintable (warning));
		Fixture f { rows };

		QCOMPARE (Sorted (f.Manager_.GetAllTags ()), Sorted (stored + QStringList { Untagged }));
		QCOMPARE (StoredNames (f.Storage_), Sorted (stored + QStringList { Untagged }));
	}

	void TagsManagerTest::testRenameRefusals_data ()
	{
		QTest::addColumn<QString> ("newName");
		QTest::addColumn<QString> ("error");

		QTest::newRow ("empty") << QString {} << "blank"_qs;
		QTest::newRow ("blank") << "   "_qs << "blank"_qs;
		QTest::newRow ("delimiter") << "a;b"_qs << "delimiter ;"_qs;
		QTest::newRow ("taken") << "taken"_qs << "taken taken"_qs;
		QTest::newRow ("taken after trimming") << " taken "_qs << "taken taken"_qs;
	}

	void TagsManagerTest::testRenameRefusals ()
	{
		QFETCH (QString, newName);
		QFETCH (QString, error);

		Fixture f;
		f.Manager_.GetID (Work);
		f.Manager_.GetID ("taken"_qs);
		const auto& idx = IndexOf (f.Manager_, Work);
		QVERIFY (idx.isValid ());
		QSignalSpy updated { &f.Manager_, &TagsManager::tagsUpdated };
		QSignalSpy changed { &f.Manager_, &QAbstractItemModel::dataChanged };

		const auto& result = f.Manager_.RenameTag (idx, newName);
		QVERIFY (result.IsLeft ());
		QCOMPARE (Describe (result.GetLeft ()), error);
		QCOMPARE (idx.data ().toString (), Work);
		QCOMPARE (StoredNames (f.Storage_), (QStringList { "taken"_qs, Untagged, Work }));
		QCOMPARE (updated.size (), 0);
		QCOMPARE (changed.size (), 0);
	}

	void TagsManagerTest::testRenameWithoutChange ()
	{
		Fixture f;
		f.Manager_.GetID (Work);
		QSignalSpy updated { &f.Manager_, &TagsManager::tagsUpdated };
		QSignalSpy changed { &f.Manager_, &QAbstractItemModel::dataChanged };

		QVERIFY (f.Manager_.RenameTag (IndexOf (f.Manager_, Work), " work "_qs).IsRight ());
		QVERIFY (f.Manager_.RenameTag ({}, "whatever"_qs).IsRight ());
		QCOMPARE (updated.size (), 0);
		QCOMPARE (changed.size (), 0);
		QCOMPARE (StoredNames (f.Storage_), (QStringList { Untagged, Work }));
	}

	void TagsManagerTest::testRename ()
	{
		Fixture f;
		const auto& id = f.Manager_.GetID (Work);
		QVERIFY (id);
		const auto& idx = IndexOf (f.Manager_, Work);
		QSignalSpy updated { &f.Manager_, &TagsManager::tagsUpdated };
		QSignalSpy changed { &f.Manager_, &QAbstractItemModel::dataChanged };

		QVERIFY (f.Manager_.RenameTag (idx, " renamed "_qs).IsRight ());
		QCOMPARE (idx.data ().toString (), "renamed"_qs);
		QCOMPARE (f.Manager_.GetTag (*id).value_or (QString {}), "renamed"_qs);
		QVERIFY (f.Manager_.GetID ("renamed"_qs) == id);
		QCOMPARE (changed.size (), 1);
		QCOMPARE (changed.first ().first ().value<QModelIndex> (), idx);
		QCOMPARE (updated.size (), 1);
		QCOMPARE (StoredNames (f.Storage_), (QStringList { "renamed"_qs, Untagged }));
	}

	void TagsManagerTest::testRemove ()
	{
		Fixture f;
		const auto& id = f.Manager_.GetID (Work);
		QVERIFY (id);
		QSignalSpy updated { &f.Manager_, &TagsManager::tagsUpdated };
		QSignalSpy removed { &f.Manager_, &QAbstractItemModel::rowsRemoved };

		f.Manager_.RemoveTag ({});
		QCOMPARE (f.Manager_.rowCount ({}), 2);
		QCOMPARE (updated.size (), 0);

		f.Manager_.RemoveTag (IndexOf (f.Manager_, Work));
		QCOMPARE (f.Manager_.rowCount ({}), 1);
		QVERIFY (!f.Manager_.GetTag (*id));
		QCOMPARE (removed.size (), 1);
		QCOMPARE (updated.size (), 1);
		QCOMPARE (StoredNames (f.Storage_), QStringList { Untagged });
	}

	void TagsManagerTest::testModelRowsFollowGetAllTags ()
	{
		Fixture f { Rows { { High, "a"_qs }, { Low, "b"_qs } } };
		const auto& tm = f.Manager_;

		const auto& names = tm.GetAllTags ();
		QCOMPARE (tm.rowCount ({}), names.size ());
		QCOMPARE (tm.columnCount ({}), 1);
		for (int i = 0; i < names.size (); ++i)
		{
			const auto& idx = tm.index (i, 0, {});
			QCOMPARE (idx.data ().toString (), names [i]);
			QVERIFY (!idx.data (Qt::EditRole).isValid ());
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

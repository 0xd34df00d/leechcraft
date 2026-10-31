/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flattofoldersproxymodeltest.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <QAbstractItemModelTester>
#include <QMimeData>
#include <QRandomGenerator>
#include <QSignalSpy>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTest>
#include "flattofoldersproxymodel.h"

QTEST_GUILESS_MAIN (LC::Util::FlatToFoldersProxyModelTest)

namespace LC::Util
{
	namespace
	{
		constexpr int IdRole = Qt::UserRole + 1;
		constexpr int KeysRole = Qt::UserRole + 2;
		const auto DropFormat = QStringLiteral ("x-test/folder");
		const auto Unkeyed = QStringLiteral ("unkeyed");

		FlatToFoldersProxyModel::Config MakeConfig ()
		{
			return
			{
				.KeysOf_ = [] (const QModelIndex& index) { return index.data (KeysRole).toStringList (); },
				.UnkeyedLabel_ = Unkeyed,
				.DropFormat_ = DropFormat,
			};
		}

		struct Source : QStandardItemModel
		{
			using QStandardItemModel::persistentIndexList;

			mutable QList<int> LastDraggedIds_;
			std::optional<QByteArray> LastDropKey_;

			void Append (int id, const QString& text, const QStringList& keys)
			{
				const auto first = new QStandardItem { text };
				first->setData (id, IdRole);
				first->setData (keys, KeysRole);
				appendRow ({ first, new QStandardItem { QString::number (id * 10) } });
			}

			void SetKeys (int row, const QStringList& keys)
			{
				item (row)->setData (keys, KeysRole);
			}

			QMimeData* mimeData (const QModelIndexList& indexes) const override
			{
				LastDraggedIds_.clear ();
				for (const auto& index : indexes)
					LastDraggedIds_ << index.data (IdRole).toInt ();
				std::ranges::sort (LastDraggedIds_);
				return QStandardItemModel::mimeData (indexes);
			}

			bool dropMimeData (const QMimeData *data, Qt::DropAction, int, int, const QModelIndex&) override
			{
				LastDropKey_ = data->hasFormat (DropFormat) ?
						std::optional { data->data (DropFormat) } :
						std::nullopt;
				return true;
			}
		};

		struct Filter : QSortFilterProxyModel
		{
			using QSortFilterProxyModel::persistentIndexList;
		};

		struct Fixture
		{
			Source Source_;
			Filter Filter_;
			QAbstractItemModel& Upstream_;
			FlatToFoldersProxyModel Proxy_ { MakeConfig () };
			QAbstractItemModelTester Tester_ { &Proxy_, QAbstractItemModelTester::FailureReportingMode::QtTest };

			explicit Fixture (bool viaFilter)
			: Upstream_ { viaFilter ? static_cast<QAbstractItemModel&> (Filter_) : Source_ }
			{
				Filter_.setSourceModel (&Source_);
			}

			void FillDefault ()
			{
				Source_.Append (1, "a", { "t1" });
				Source_.Append (2, "b", { "t1", "t2" });
				Source_.Append (3, "c", {});
			}

			void Attach ()
			{
				Proxy_.SetSourceModel (&Upstream_);
			}

			int UpstreamPersistentCount () const
			{
				return (&Upstream_ == &Filter_ ? Filter_.persistentIndexList () : Source_.persistentIndexList ()).size ();
			}
		};

		struct Spies
		{
			QSignalSpy Inserted_;
			QSignalSpy Removed_;
			QSignalSpy Reset_;
			QSignalSpy Layout_;
			QSignalSpy Data_;

			explicit Spies (QAbstractItemModel& model)
			: Inserted_ { &model, &QAbstractItemModel::rowsInserted }
			, Removed_ { &model, &QAbstractItemModel::rowsRemoved }
			, Reset_ { &model, &QAbstractItemModel::modelReset }
			, Layout_ { &model, &QAbstractItemModel::layoutChanged }
			, Data_ { &model, &QAbstractItemModel::dataChanged }
			{
			}

			int Structural () const
			{
				return Inserted_.size () + Removed_.size () + Reset_.size () + Layout_.size ();
			}
		};

		QStringList KeysOf (const QModelIndex& index)
		{
			auto keys = index.data (KeysRole).toStringList ();
			keys.removeDuplicates ();
			if (keys.isEmpty ())
				keys << Unkeyed;
			return keys;
		}

		QString FolderName (const QModelIndex& folder)
		{
			return folder.data ().toString ();
		}

		using Picture = QMap<QString, QSet<int>>;

		Picture Expected (const QAbstractItemModel& upstream)
		{
			Picture result;
			for (int row = 0; row < upstream.rowCount (); ++row)
			{
				const auto& index = upstream.index (row, 0);
				for (const auto& key : KeysOf (index))
					result [key] << index.data (IdRole).toInt ();
			}
			return result;
		}

		QString Describe (const Picture& picture)
		{
			QStringList folders;
			for (const auto& [key, ids] : picture.asKeyValueRange ())
			{
				auto sorted = ids.values ();
				std::ranges::sort (sorted);
				QStringList idStrings;
				for (const auto id : sorted)
					idStrings << QString::number (id);
				folders << key + " {" + idStrings.join (' ') + '}';
			}
			return folders.join ("; ");
		}

		QStringList Dump (const FlatToFoldersProxyModel& proxy)
		{
			QStringList result;
			for (int f = 0; f < proxy.rowCount (); ++f)
			{
				const auto& folder = proxy.index (f, 0);
				QStringList ids;
				for (int l = 0; l < proxy.rowCount (folder); ++l)
					ids << QString::number (proxy.index (l, 0, folder).data (IdRole).toInt ());
				result << FolderName (folder) + ": " + ids.join (' ');
			}
			return result;
		}

		void CheckTree (const FlatToFoldersProxyModel& proxy, const QAbstractItemModel& upstream)
		{
			QCOMPARE (proxy.columnCount (), upstream.columnCount ());

			Picture actual;
			int leaves = 0;
			for (int f = 0; f < proxy.rowCount (); ++f)
			{
				const auto& folderIdx = proxy.index (f, 0);
				QVERIFY (proxy.IsFolder (folderIdx));
				QCOMPARE (proxy.parent (folderIdx), QModelIndex {});
				if (upstream.columnCount () > 1)
					QCOMPARE (proxy.rowCount (folderIdx.siblingAtColumn (1)), 0);

				const auto& key = FolderName (folderIdx);
				QVERIFY2 (!actual.contains (key), qPrintable ("duplicate folder for key " + key));
				QVERIFY2 (proxy.rowCount (folderIdx) > 0, qPrintable ("empty folder for key " + key));

				auto& ids = actual [key];
				for (int l = 0; l < proxy.rowCount (folderIdx); ++l)
				{
					++leaves;
					const auto& leafIdx = proxy.index (l, 0, folderIdx);
					QVERIFY (!proxy.IsFolder (leafIdx));
					QCOMPARE (proxy.rowCount (leafIdx), 0);
					QCOMPARE (proxy.parent (leafIdx), folderIdx);

					const auto& source = proxy.MapToSource (leafIdx);
					QVERIFY (source.isValid ());
					QCOMPARE (source.model (), &upstream);
					QVERIFY (KeysOf (source).contains (key));
					QVERIFY (proxy.MapFromSource (source).contains (leafIdx));
					for (int col = 0; col < upstream.columnCount (); ++col)
						QCOMPARE (proxy.index (l, col, folderIdx).data (), source.siblingAtColumn (col).data ());

					ids << leafIdx.data (IdRole).toInt ();
				}
			}

			const auto& expected = Expected (upstream);
			QVERIFY2 (actual == expected,
					qPrintable ("tree is " + Describe (actual) + ", source says " + Describe (expected)));

			int expectedLeaves = 0;
			for (int row = 0; row < upstream.rowCount (); ++row)
				expectedLeaves += KeysOf (upstream.index (row, 0)).size ();
			QCOMPARE (leaves, expectedLeaves);
		}

		QModelIndex Parent (const QList<QVariant>& signalArgs)
		{
			return signalArgs.at (0).value<QModelIndex> ();
		}
	}

	void FlatToFoldersProxyModelTest::initTestCase ()
	{
		// The fixture attaches its own tester in QtTest reporting mode.
		qunsetenv ("LC_MODEL_SANITIZER_MODE");
	}

	void FlatToFoldersProxyModelTest::initTestCase_data ()
	{
		QTest::addColumn<bool> ("viaFilter");
		QTest::newRow ("direct source") << false;
		QTest::newRow ("through a QSortFilterProxyModel") << true;
	}

	void FlatToFoldersProxyModelTest::testInitialTree ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		CheckTree (f.Proxy_, f.Upstream_);

		QCOMPARE (Dump (f.Proxy_), (QStringList { "t1: 1 2", "t2: 2", "unkeyed: 3" }));
		QCOMPARE (f.Proxy_.headerData (0, Qt::Horizontal, Qt::DisplayRole), f.Upstream_.headerData (0, Qt::Horizontal, Qt::DisplayRole));
		QCOMPARE (f.Proxy_.flags ({}), Qt::ItemIsDropEnabled);
	}

	void FlatToFoldersProxyModelTest::testFoldersFollowKeys ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.Source_.Append (1, "a", { "t1" });
		f.Source_.Append (2, "b", { "t1" });
		f.Source_.Append (3, "c", { "x", "y" });
		f.Source_.Append (4, "d", {});
		f.Source_.Append (5, "e", { "t1", "t1" });
		f.Attach ();
		CheckTree (f.Proxy_, f.Upstream_);
		QCOMPARE (Dump (f.Proxy_), (QStringList { "t1: 1 2 5", "x: 3", "y: 3", "unkeyed: 4" }));
		Spies spies { f.Proxy_ };

		f.Source_.SetKeys (3, { "t1" });
		QCOMPARE (spies.Inserted_.size (), 1);
		QCOMPARE (spies.Removed_.size (), 2);
		CheckTree (f.Proxy_, f.Upstream_);
		QCOMPARE (Dump (f.Proxy_), (QStringList { "t1: 1 2 5 4", "x: 3", "y: 3" }));
	}

	void FlatToFoldersProxyModelTest::testInsertCreatesFoldersOnDemand ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		Spies spies { f.Proxy_ };

		f.Source_.Append (4, "d", { "t3" });
		QCOMPARE (spies.Inserted_.size (), 2);
		QCOMPARE (Parent (spies.Inserted_.at (0)), QModelIndex {});
		QVERIFY (Parent (spies.Inserted_.at (1)).isValid ());

		f.Source_.Append (5, "e", { "t1" });
		QCOMPARE (spies.Inserted_.size (), 3);
		QCOMPARE (spies.Structural (), 3);
		CheckTree (f.Proxy_, f.Upstream_);
	}

	void FlatToFoldersProxyModelTest::testRekeyKeepsTheSurvivingLeaf ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		Spies spies { f.Proxy_ };

		f.Source_.SetKeys (1, { "t2", "t3" });

		QCOMPARE (spies.Data_.size (), 1);
		const auto& args = spies.Data_.first ();
		const auto& topLeft = args.at (0).value<QModelIndex> ();
		QCOMPARE (topLeft.data (IdRole).toInt (), 2);
		QCOMPARE (FolderName (topLeft.parent ()), QString { "t2" });
		QCOMPARE (args.at (1).value<QModelIndex> ().column (), f.Upstream_.columnCount () - 1);
		QCOMPARE (args.at (2).value<QList<int>> (), (QList<int> { KeysRole }));

		QCOMPARE (spies.Removed_.size (), 1);
		QCOMPARE (spies.Inserted_.size (), 2);
		CheckTree (f.Proxy_, f.Upstream_);
	}

	void FlatToFoldersProxyModelTest::testRemovingTheLastLeafRemovesTheFolder ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		Spies spies { f.Proxy_ };

		f.Source_.removeRow (2);

		QCOMPARE (spies.Removed_.size (), 2);
		QVERIFY (Parent (spies.Removed_.at (0)).isValid ());
		QCOMPARE (Parent (spies.Removed_.at (1)), QModelIndex {});
		QCOMPARE (f.Proxy_.rowCount (), 2);
		CheckTree (f.Proxy_, f.Upstream_);
	}

	void FlatToFoldersProxyModelTest::testRemovalFollowsOwnBookkeeping ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();

		f.Source_.blockSignals (true);
		f.Source_.SetKeys (2, { "zz" });
		f.Source_.blockSignals (false);
		f.Source_.removeRow (2);

		QCOMPARE (f.Proxy_.rowCount (), 2);
		CheckTree (f.Proxy_, f.Upstream_);
	}

	void FlatToFoldersProxyModelTest::testSourceSortLeavesTheTreeAlone ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		const auto& before = Dump (f.Proxy_);
		Spies spies { f.Proxy_ };

		f.Source_.sort (0, Qt::DescendingOrder);

		QCOMPARE (spies.Layout_.size (), 0);
		QCOMPARE (spies.Structural (), 0);
		QCOMPARE (Dump (f.Proxy_), before);
		CheckTree (f.Proxy_, f.Upstream_);
	}

	void FlatToFoldersProxyModelTest::testSourceResetIsOneReset ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		Spies spies { f.Proxy_ };

		f.Source_.clear ();
		QCOMPARE (spies.Reset_.size (), 1);
		QCOMPARE (spies.Structural (), 1);
		QCOMPARE (f.Proxy_.rowCount (), 0);

		f.Source_.Append (4, "d", { "t1", "t2" });
		QCOMPARE (spies.Inserted_.size (), 4);
		CheckTree (f.Proxy_, f.Upstream_);
	}

	void FlatToFoldersProxyModelTest::testAttachAndDetach ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		Spies spies { f.Proxy_ };

		f.Attach ();
		QCOMPARE (spies.Reset_.size (), 1);
		QCOMPARE (spies.Structural (), 1);
		QCOMPARE (f.Proxy_.rowCount (), 3);
		CheckTree (f.Proxy_, f.Upstream_);

		f.Proxy_.SetSourceModel (nullptr);
		QCOMPARE (spies.Reset_.size (), 2);
		QCOMPARE (spies.Structural (), 2);
		QCOMPARE (f.Proxy_.rowCount (), 0);
		QCOMPARE (f.Proxy_.columnCount (), 0);
		QCOMPARE (f.Proxy_.GetSourceModel (), nullptr);

		f.Source_.Append (4, "d", { "t9" });
		QCOMPARE (spies.Structural (), 2);
	}

	void FlatToFoldersProxyModelTest::testDragAndDropMapThroughFolders ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		const auto& t1 = f.Proxy_.index (0, 0);

		std::unique_ptr<QMimeData> folderDrag { f.Proxy_.mimeData ({ t1 }) };
		QVERIFY (folderDrag);
		QCOMPARE (f.Source_.LastDraggedIds_, (QList<int> { 1, 2 }));

		std::unique_ptr<QMimeData> leafDrag { f.Proxy_.mimeData ({ f.Proxy_.index (1, 0, t1) }) };
		QVERIFY (leafDrag);
		QCOMPARE (f.Source_.LastDraggedIds_, (QList<int> { 2 }));

		QMimeData payload;
		payload.setText ("payload");
		QVERIFY (f.Proxy_.dropMimeData (&payload, Qt::CopyAction, -1, -1, t1));
		QVERIFY (f.Source_.LastDropKey_.has_value ());
		QCOMPARE (*f.Source_.LastDropKey_, QByteArray { "t1" });

		QVERIFY (f.Proxy_.dropMimeData (&payload, Qt::CopyAction, -1, -1, f.Proxy_.index (2, 0)));
		QVERIFY (f.Source_.LastDropKey_.has_value ());
		QVERIFY (f.Source_.LastDropKey_->isEmpty ());

		QVERIFY (f.Proxy_.dropMimeData (&payload, Qt::CopyAction, -1, -1, {}));
		QVERIFY (!f.Source_.LastDropKey_.has_value ());
	}

	void FlatToFoldersProxyModelTest::testReleasesUpstreamIndexes ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		const auto baseline = f.UpstreamPersistentCount ();

		Source other;
		other.Append (7, "x", { "t1" });
		{
			FlatToFoldersProxyModel proxy { MakeConfig () };
			QAbstractItemModelTester tester { &proxy, QAbstractItemModelTester::FailureReportingMode::QtTest };

			proxy.SetSourceModel (&f.Upstream_);
			QCOMPARE (f.UpstreamPersistentCount (), baseline + 3);

			proxy.SetSourceModel (&other);
			QCOMPARE (f.UpstreamPersistentCount (), baseline);

			proxy.SetSourceModel (&f.Upstream_);
			QCOMPARE (f.UpstreamPersistentCount (), baseline + 3);
		}
		QCOMPARE (f.UpstreamPersistentCount (), baseline);
	}

	void FlatToFoldersProxyModelTest::testRandomWalk ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.Attach ();

		constexpr quint32 seed = 20261017;
		QRandomGenerator gen { seed };
		const QStringList pool { "t1", "t2", "t3", "t4" };
		const auto randomKeys = [&]
		{
			QStringList keys;
			for (const auto& key : pool)
				if (!gen.bounded (3))
					keys << key;
			if (!keys.isEmpty () && !gen.bounded (4))
				keys << keys.first ();
			return keys;
		};
		const auto randomText = [&] { return QString { QChar { 'a' + static_cast<int> (gen.bounded (6)) } }; };

		int nextId = 1;
		for (int step = 0; step < 400; ++step)
		{
			const auto rows = f.Source_.rowCount ();
			switch (rows ? gen.bounded (6) : 0)
			{
			case 0:
				f.Source_.Append (nextId++, randomText (), randomKeys ());
				break;
			case 1:
				f.Source_.removeRow (gen.bounded (rows));
				break;
			case 2:
				f.Source_.SetKeys (gen.bounded (rows), randomKeys ());
				break;
			case 3:
				f.Source_.item (gen.bounded (rows))->setText (randomText ());
				break;
			case 4:
				if (gen.bounded (2))
					f.Source_.sort (0, gen.bounded (2) ? Qt::AscendingOrder : Qt::DescendingOrder);
				else
					f.Filter_.setFilterFixedString (gen.bounded (2) ? QString {} : randomText ());
				break;
			case 5:
				if (!gen.bounded (20))
					f.Source_.clear ();
				break;
			}

			CheckTree (f.Proxy_, f.Upstream_);
			if (QTest::currentTestFailed ())
			{
				qWarning () << "random walk failed at step" << step << "with seed" << seed << "upstream:" << Describe (Expected (f.Upstream_));
				return;
			}
		}
	}
}

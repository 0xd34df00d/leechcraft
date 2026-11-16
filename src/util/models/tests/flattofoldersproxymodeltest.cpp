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
#include <QHash>
#include <QMimeData>
#include <QSignalSpy>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTest>
#include <util/test/collectingmodeltester.h>
#include <util/test/randomwalk.h>
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
		const QStringList InitialKeys { "t1", "t2", "t3", "t4" };

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

		// the keys stored in the source rows map to themselves unless Aliases_ says otherwise,
		// which is how a key gets replaced behind the source's back
		struct Models
		{
			QHash<QString, QStringList> Aliases_;
			Source Source_;
			Filter Filter_;
			QAbstractItemModel& Upstream_;
			FlatToFoldersProxyModel Proxy_ { MakeConfig () };

			explicit Models (bool viaFilter)
			: Upstream_ { viaFilter ? static_cast<QAbstractItemModel&> (Filter_) : Source_ }
			{
				Filter_.setSourceModel (&Source_);
			}

			FlatToFoldersProxyModel::Config MakeConfig () const
			{
				return
				{
					.KeysOf_ = [this] (const QModelIndex& index) { return KeysOf (index); },
					.UnkeyedLabel_ = Unkeyed,
					.DropFormat_ = DropFormat,
				};
			}

			QStringList KeysOf (const QModelIndex& index) const
			{
				QStringList keys;
				for (const auto& stored : index.data (KeysRole).toStringList ())
					keys += Aliases_.value (stored, { stored });
				return keys;
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

			void ReplaceKey (const QString& old, const QStringList& newKeys)
			{
				for (int row = 0; row < Source_.rowCount (); ++row)
					for (const auto& stored : Source_.index (row, 0).data (KeysRole).toStringList ())
					{
						auto keys = Aliases_.value (stored, { stored });
						if (!keys.contains (old))
							continue;

						keys.removeAll (old);
						for (const auto& key : newKeys)
							if (!keys.contains (key))
								keys << key;
						Aliases_ [stored] = keys;
					}
				Proxy_.ReplaceKey (old, newKeys);
			}

			int UpstreamPersistentCount () const
			{
				return (&Upstream_ == &Filter_ ? Filter_.persistentIndexList () : Source_.persistentIndexList ()).size ();
			}
		};

		struct Fixture : Models
		{
			QAbstractItemModelTester Tester_ { &Proxy_, QAbstractItemModelTester::FailureReportingMode::QtTest };

			using Models::Models;
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

		QStringList ExpectedKeysOf (const Models& models, const QModelIndex& index)
		{
			auto keys = models.KeysOf (index);
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

		Picture Expected (const Models& models)
		{
			Picture result;
			for (int row = 0; row < models.Upstream_.rowCount (); ++row)
			{
				const auto& index = models.Upstream_.index (row, 0);
				for (const auto& key : ExpectedKeysOf (models, index))
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

		std::optional<QString> TreeViolation (const Models& models)
		{
			const auto& proxy = models.Proxy_;
			const auto& upstream = models.Upstream_;
			if (proxy.columnCount () != upstream.columnCount ())
				return QString { "the tree has %1 columns, the source has %2" }.arg (proxy.columnCount ()).arg (upstream.columnCount ());

			Picture actual;
			int leaves = 0;
			for (int f = 0; f < proxy.rowCount (); ++f)
			{
				const auto& folderIdx = proxy.index (f, 0);
				if (!proxy.IsFolder (folderIdx))
					return QString { "top-level row %1 is not a folder" }.arg (f);

				const auto& key = FolderName (folderIdx);
				if (proxy.parent (folderIdx).isValid ())
					return "folder " + key + " has a parent";
				if (upstream.columnCount () > 1 && proxy.rowCount (folderIdx.siblingAtColumn (1)))
					return "folder " + key + " has rows under its second column";
				if (actual.contains (key))
					return "duplicate folder for key " + key;
				if (!proxy.rowCount (folderIdx))
					return "empty folder for key " + key;

				auto& ids = actual [key];
				for (int l = 0; l < proxy.rowCount (folderIdx); ++l)
				{
					++leaves;
					const auto aboutLeaf = [&] (const QString& problem) { return QString { "leaf %1 of folder %2 %3" }.arg (l).arg (key, problem); };

					const auto& leafIdx = proxy.index (l, 0, folderIdx);
					if (proxy.IsFolder (leafIdx))
						return aboutLeaf ("is a folder");
					if (proxy.rowCount (leafIdx))
						return aboutLeaf ("has rows under it");
					if (proxy.parent (leafIdx) != folderIdx)
						return aboutLeaf ("has another parent");

					const auto& source = proxy.MapToSource (leafIdx);
					if (!source.isValid ())
						return aboutLeaf ("maps to no source row");
					if (source.model () != &upstream)
						return aboutLeaf ("maps to another model");
					if (!ExpectedKeysOf (models, source).contains (key))
						return aboutLeaf ("maps to a source row without this key");
					if (!proxy.MapFromSource (source).contains (leafIdx))
						return aboutLeaf ("is not among what its source row maps to");
					for (int col = 0; col < upstream.columnCount (); ++col)
						if (proxy.index (l, col, folderIdx).data () != source.siblingAtColumn (col).data ())
							return aboutLeaf (QString { "differs from its source row in column %1" }.arg (col));

					ids << leafIdx.data (IdRole).toInt ();
				}
			}

			const auto& expected = Expected (models);
			if (actual != expected)
				return "tree is " + Describe (actual) + ", source says " + Describe (expected);

			int expectedLeaves = 0;
			for (int row = 0; row < upstream.rowCount (); ++row)
				expectedLeaves += ExpectedKeysOf (models, upstream.index (row, 0)).size ();
			if (leaves != expectedLeaves)
				return QString { "the tree has %1 leaves, the source rows need %2" }.arg (leaves).arg (expectedLeaves);

			return {};
		}

		void CheckTree (const Models& models)
		{
			if (const auto violation = TreeViolation (models))
				QFAIL (qPrintable (*violation));
		}

		QModelIndex Parent (const QList<QVariant>& signalArgs)
		{
			return signalArgs.at (0).value<QModelIndex> ();
		}

		template<bool ViaFilter>
		struct TreeWalk
		{
			Models Models_ { ViaFilter };
			CollectingModelTester Tester_ { Models_.Proxy_ };
			QStringList KnownKeys_ = InitialKeys;
			int NextId_ = 1;

			TreeWalk ()
			{
				Models_.Attach ();
			}

			void Append (Choices& choices)
			{
				const auto& text = DrawText (choices);
				Models_.Source_.Append (NextId_++, text, DrawKeys (choices));
			}

			void Remove (Choices& choices)
			{
				if (const auto rows = Models_.Source_.rowCount ())
					Models_.Source_.removeRow (choices.Draw (rows));
			}

			void SetKeys (Choices& choices)
			{
				if (const auto rows = Models_.Source_.rowCount ())
				{
					const auto row = choices.Draw (rows);
					Models_.Source_.SetKeys (row, DrawKeys (choices));
				}
			}

			void SetText (Choices& choices)
			{
				if (const auto rows = Models_.Source_.rowCount ())
				{
					const auto row = choices.Draw (rows);
					Models_.Source_.item (row)->setText (DrawText (choices));
				}
			}

			void Sort (Choices& choices)
			{
				Models_.Source_.sort (0, choices.Draw (2) ? Qt::DescendingOrder : Qt::AscendingOrder);
			}

			void SetFilter (Choices& choices)
			{
				Models_.Filter_.setFilterFixedString (choices.Draw (2) ? DrawText (choices) : QString {});
			}

			void Clear (Choices& choices)
			{
				if (!choices.Draw (20))
					Models_.Source_.clear ();
			}

			void ReplaceKey (Choices& choices)
			{
				const auto old = KnownKeys_ [choices.Draw (KnownKeys_.size ())];
				QStringList newKeys;
				for (const auto& key : std::as_const (KnownKeys_))
					if (choices.Draw (3) == 2)
						newKeys << key;
				if (choices.Draw (2))
					newKeys << KnownKeys_.emplaceBack ("r" + QString::number (KnownKeys_.size ()));
				Models_.ReplaceKey (old, newKeys);
			}

			std::optional<QString> Check () const
			{
				if (const auto& failures = Tester_.GetFailures ();
					!failures.isEmpty ())
					return failures.first ();
				return TreeViolation (Models_);
			}
		private:
			static QString DrawText (Choices& choices)
			{
				return QString { QChar { 'a' + choices.Draw (6) } };
			}

			static QStringList DrawKeys (Choices& choices)
			{
				QStringList keys;
				for (const auto& key : InitialKeys)
					if (choices.Draw (3) == 2)
						keys << key;
				if (!keys.isEmpty () && choices.Draw (4) == 3)
					keys << keys.first ();
				return keys;
			}
		};

		template<bool ViaFilter>
		void RunTreeWalks ()
		{
			using Walk = TreeWalk<ViaFilter>;
			RunRandomWalks<Walk> ({ .Seed_ = 20261017, .Walks_ = 300, .Steps_ = 60 },
					{
						{ "append", &Walk::Append },
						{ "remove", &Walk::Remove },
						{ "set keys", &Walk::SetKeys },
						{ "set text", &Walk::SetText },
						{ "sort", &Walk::Sort },
						{ "set filter", &Walk::SetFilter },
						{ "clear", &Walk::Clear },
						{ "replace key", &Walk::ReplaceKey },
					});
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
		CheckTree (f);

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
		CheckTree (f);
		QCOMPARE (Dump (f.Proxy_), (QStringList { "t1: 1 2 5", "x: 3", "y: 3", "unkeyed: 4" }));
		Spies spies { f.Proxy_ };

		f.Source_.SetKeys (3, { "t1" });
		QCOMPARE (spies.Inserted_.size (), 1);
		QCOMPARE (spies.Removed_.size (), 2);
		CheckTree (f);
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
		CheckTree (f);
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
		CheckTree (f);
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
		CheckTree (f);
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
		CheckTree (f);
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
		CheckTree (f);
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
		CheckTree (f);
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
		CheckTree (f);

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
			FlatToFoldersProxyModel proxy { f.MakeConfig () };
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

	void FlatToFoldersProxyModelTest::testRenameKeepsTheFolder ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		Spies spies { f.Proxy_ };

		f.ReplaceKey ("t1", { "renamed" });

		QCOMPARE (spies.Structural (), 0);
		QCOMPARE (spies.Data_.size (), 1);
		const auto& args = spies.Data_.first ();
		QCOMPARE (args.at (0).value<QModelIndex> (), f.Proxy_.index (0, 0));
		QCOMPARE (args.at (1).value<QModelIndex> (), f.Proxy_.index (0, 0));
		QCOMPARE (args.at (2).value<QList<int>> (), (QList<int> { Qt::DisplayRole }));
		QCOMPARE (Dump (f.Proxy_), (QStringList { "renamed: 1 2", "t2: 2", "unkeyed: 3" }));
		CheckTree (f);
	}

	void FlatToFoldersProxyModelTest::testRenameOntoAnExistingKeyMerges ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		Spies spies { f.Proxy_ };

		f.ReplaceKey ("t1", { "t2" });

		QCOMPARE (spies.Inserted_.size (), 1);
		QCOMPARE (spies.Removed_.size (), 3);
		QCOMPARE (Dump (f.Proxy_), (QStringList { "t2: 2 1", "unkeyed: 3" }));
		CheckTree (f);
	}

	void FlatToFoldersProxyModelTest::testSplitSpreadsTheRows ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();

		f.ReplaceKey ("t1", { "x", "y" });

		QCOMPARE (Dump (f.Proxy_), (QStringList { "t2: 2", "unkeyed: 3", "x: 1 2", "y: 1 2" }));
		CheckTree (f);
	}

	void FlatToFoldersProxyModelTest::testRemovedKeyMovesRowsToUnkeyed ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();

		f.ReplaceKey ("t1", {});

		QCOMPARE (Dump (f.Proxy_), (QStringList { "t2: 2", "unkeyed: 3 1" }));
		CheckTree (f);
	}

	void FlatToFoldersProxyModelTest::testKeepingTheOldKeyAddsFolders ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();

		f.ReplaceKey ("t2", { "t2", "t3" });

		QCOMPARE (Dump (f.Proxy_), (QStringList { "t1: 1 2", "t2: 2", "unkeyed: 3", "t3: 2" }));
		CheckTree (f);
	}

	void FlatToFoldersProxyModelTest::testUnknownKeyChangesNothing ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		Fixture f { viaFilter };
		f.FillDefault ();
		f.Attach ();
		Spies spies { f.Proxy_ };

		f.ReplaceKey ("nope", { "t1" });

		QCOMPARE (spies.Structural (), 0);
		QCOMPARE (spies.Data_.size (), 0);
		QCOMPARE (Dump (f.Proxy_), (QStringList { "t1: 1 2", "t2: 2", "unkeyed: 3" }));
	}

	void FlatToFoldersProxyModelTest::testRandomWalks ()
	{
		QFETCH_GLOBAL (bool, viaFilter);
		if (viaFilter)
			RunTreeWalks<true> ();
		else
			RunTreeWalks<false> ();
	}
}

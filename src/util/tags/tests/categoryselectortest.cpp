/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "categoryselectortest.h"
#include <QAction>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTreeView>
#include <QtTest>
#include <util/sll/qtutil.h>
#include "categoryselector.h"

QTEST_MAIN (LC::Util::CategorySelectorTest)

namespace LC::Util
{
	namespace
	{
		const auto A = "a"_qs;
		const auto B = "b"_qs;
		const auto C = "C"_qs;

		struct Fixture
		{
			QStandardItemModel Model_;
			CategorySelector Selector_;
			QSignalSpy Edited_ { &Selector_, &CategorySelector::selectionEdited };

			explicit Fixture (const QStringList& names = { B, A, C })
			{
				for (const auto& name : names)
					Model_.appendRow (new QStandardItem { name });
				Selector_.SetModel (Model_);
			}

			QTreeView& View () const
			{
				return *Selector_.findChild<QTreeView*> ();
			}

			QStringList Listed () const
			{
				QStringList result;
				const auto model = View ().model ();
				for (int i = 0; i < model->rowCount (); ++i)
					result << model->index (i, 0).data ().toString ();
				return result;
			}

			bool Check (const QString& name, Qt::CheckState state) const
			{
				const auto model = View ().model ();
				const auto& found = model->match (model->index (0, 0), Qt::DisplayRole, name, 1, Qt::MatchExactly);
				return !found.isEmpty () && model->setData (found.front (), state, Qt::CheckStateRole);
			}

			QStringList LastReported () const
			{
				return Edited_.last ().first ().toStringList ();
			}
		};
	}

	void CategorySelectorTest::initMain ()
	{
		qputenv ("QT_QPA_PLATFORM", "offscreen");
	}

	void CategorySelectorTest::testListsTheNamesSorted ()
	{
		Fixture f;
		QCOMPARE (f.Listed (), (QStringList { A, B, C }));

		f.Selector_.SelectAll ();
		QCOMPARE (f.Selector_.GetSelections (), (QStringList { A, B, C }));
		QCOMPARE (f.Edited_.size (), 0);
	}

	void CategorySelectorTest::testSelectionIsKeyedByName ()
	{
		Fixture f;
		f.Selector_.SetSelections ({ A, "zzz"_qs });
		QCOMPARE (f.Selector_.GetSelections (), QStringList { A });

		f.Model_.appendRow (new QStandardItem { "zzz"_qs });
		QCOMPARE (f.Selector_.GetSelections (), (QStringList { A, "zzz"_qs }));

		f.Model_.clear ();
		QVERIFY (f.Selector_.GetSelections ().isEmpty ());

		f.Model_.appendRow (new QStandardItem { A });
		QCOMPARE (f.Selector_.GetSelections (), QStringList { A });
		QCOMPARE (f.Edited_.size (), 0);
	}

	void CategorySelectorTest::testSelectedIndexesAreTheModelsOwn ()
	{
		Fixture f;
		f.Model_.item (1)->setData (42, Qt::UserRole);
		f.Selector_.SetSelections ({ A });

		const auto& indexes = f.Selector_.GetSelectedIndexes ();
		QCOMPARE (indexes.size (), 1);
		QCOMPARE (indexes.front ().model (), &f.Model_);
		QCOMPARE (indexes.front ().data (Qt::UserRole).toInt (), 42);
	}

	void CategorySelectorTest::testUserChecksAreReported ()
	{
		Fixture f;
		QVERIFY (f.Check (B, Qt::Checked));
		QCOMPARE (f.Edited_.size (), 1);
		QCOMPARE (f.LastReported (), QStringList { B });

		QVERIFY (f.Check (A, Qt::Checked));
		QCOMPARE (f.Edited_.size (), 2);
		QCOMPARE (f.LastReported (), (QStringList { A, B }));

		QVERIFY (f.Check (B, Qt::Unchecked));
		QCOMPARE (f.Edited_.size (), 3);
		QCOMPARE (f.LastReported (), QStringList { A });

		f.Selector_.SelectNone ();
		f.Selector_.SetSelections ({ C });
		QCOMPARE (f.Edited_.size (), 3);
		QCOMPARE (f.Selector_.GetSelections (), QStringList { C });
	}

	void CategorySelectorTest::testContextActionsAreReported ()
	{
		Fixture f;
		const auto& actions = f.View ().actions ();
		QCOMPARE (actions.size (), 2);

		actions.first ()->trigger ();
		QCOMPARE (f.Selector_.GetSelections (), (QStringList { A, B, C }));
		QCOMPARE (f.Edited_.size (), 1);
		QCOMPARE (f.LastReported (), (QStringList { A, B, C }));

		actions.last ()->trigger ();
		QVERIFY (f.Selector_.GetSelections ().isEmpty ());
		QCOMPARE (f.Edited_.size (), 2);
		QVERIFY (f.LastReported ().isEmpty ());
	}

	void CategorySelectorTest::testCaptionShowsTheHeader ()
	{
		Fixture f;
		QVERIFY (f.View ().isHeaderHidden ());

		f.Selector_.SetCaption ("Tags"_qs);
		QVERIFY (!f.View ().isHeaderHidden ());
		QCOMPARE (f.View ().model ()->headerData (0, Qt::Horizontal).toString (), "Tags"_qs);

		f.Selector_.SetCaption ({});
		QVERIFY (f.View ().isHeaderHidden ());
	}

	void CategorySelectorTest::testSetModelKeepsTheSelection ()
	{
		Fixture f;
		f.Selector_.SetSelections ({ A, B });

		QStandardItemModel other;
		for (const auto& name : { "x"_qs, A })
			other.appendRow (new QStandardItem { name });
		f.Selector_.SetModel (other);

		QCOMPARE (f.Listed (), (QStringList { A, "x"_qs }));
		QCOMPARE (f.Selector_.GetSelections (), QStringList { A });
		QCOMPARE (f.Edited_.size (), 0);
	}
}

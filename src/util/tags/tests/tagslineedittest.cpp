/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagslineedittest.h"
#include <utility>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTreeView>
#include <QtTest>
#include <util/sll/qtutil.h>
#include "categoryselector.h"
#include "tagschange.h"
#include "tagscompleter.h"
#include "tagslineedit.h"

QTEST_MAIN (LC::Util::TagsLineEditTest)

namespace LC::Util
{
	namespace
	{
		const auto A = "a"_qs;
		const auto B = "b"_qs;
		const auto C = "c"_qs;
		const auto Z = "z"_qs;

		struct Fixture
		{
			QStandardItemModel TagsModel_;
			TagsLineEdit Edit_ { nullptr };
			TagsCompleter& Completer_ = *new TagsCompleter { &Edit_ };
			QList<TagsChange> Edits_;

			Fixture ()
			{
				for (const auto& tag : { A, B, C })
					TagsModel_.appendRow (new QStandardItem { tag });

				Completer_.OverrideModel (&TagsModel_);
				Edit_.AddSelector ();
				QObject::connect (&Edit_,
						&TagsLineEdit::tagsEdited,
						[this] (const TagsChange& change) { Edits_ << change; });
			}

			CategorySelector& Selector () const
			{
				return *Edit_.findChild<CategorySelector*> ();
			}

			bool SetChecked (const QString& tag, Qt::CheckState state) const
			{
				const auto model = Selector ().findChild<QTreeView*> ()->model ();
				const auto& found = model->match (model->index (0, 0), Qt::DisplayRole, tag, 1, Qt::MatchExactly);
				return !found.isEmpty () && model->setData (found.front (), state, Qt::CheckStateRole);
			}

			void FinishEditing ()
			{
				emit Edit_.editingFinished ();
			}

			void Complete (const QString& completion)
			{
				emit Completer_.activated (completion);
			}
		};
	}

	void TagsLineEditTest::testSelectorChoiceIsReported ()
	{
		Fixture f;
		f.Edit_.SetTags ({ A, B });

		QSignalSpy chosen { &f.Edit_, &TagsLineEdit::tagsChosen };
		QVERIFY (f.SetChecked (B, Qt::Unchecked));

		QCOMPARE (f.Edit_.text (), A);
		QCOMPARE (chosen.size (), 1);
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, (QStringList { A, B }));
		QCOMPARE (f.Edits_.last ().After_, QStringList { A });
	}

	void TagsLineEditTest::testSelectorFollowsNonCanonicalText ()
	{
		Fixture f;
		f.Edit_.setText ("a;b"_qs);
		QCOMPARE (f.Selector ().GetSelections (), (QStringList { A, B }));

		QVERIFY (f.SetChecked (C, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "a; b; c"_qs);
	}

	void TagsLineEditTest::testSetTagsSyncsTheSelectorWithBlockedSignals ()
	{
		Fixture f;
		f.Edit_.SetTags ({ A });
		{
			const QSignalBlocker blocker { &f.Edit_ };
			f.Edit_.SetTags ({ B });
		}
		QCOMPARE (f.Selector ().GetSelections (), QStringList { B });

		QVERIFY (f.SetChecked (C, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "b; c"_qs);
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, QStringList { B });
		QCOMPARE (f.Edits_.last ().After_, (QStringList { B, C }));
	}

	void TagsLineEditTest::testNestedSetTagsWins ()
	{
		Fixture f;

		// Like a text change handler answering with tags of its own.
		auto nested = false;
		QObject::connect (&f.Edit_, &QLineEdit::textChanged, [&]
				{
					if (!std::exchange (nested, true))
						f.Edit_.SetTags ({ B });
				});
		f.Edit_.SetTags ({ A });
		QCOMPARE (f.Edit_.text (), B);
		QCOMPARE (f.Selector ().GetSelections (), QStringList { B });

		QVERIFY (f.SetChecked (C, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "b; c"_qs);
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, QStringList { B });
	}

	void TagsLineEditTest::testUnchangedTagsAreNotReported ()
	{
		Fixture f;
		f.Edit_.SetTags ({ A, B });

		f.FinishEditing ();
		f.Edit_.setText ("b; a"_qs);
		f.FinishEditing ();
		f.Edit_.setText ("a;b"_qs);
		f.FinishEditing ();
		QVERIFY (f.Edits_.isEmpty ());
	}

	void TagsLineEditTest::testSetTagsIsNeverReported ()
	{
		Fixture f;
		f.Edit_.SetTags ({ A });

		// Like a text change handler taking the focus away.
		QObject::connect (&f.Edit_, &QLineEdit::textChanged, [&f] { f.FinishEditing (); });
		f.Edit_.SetTags ({ B });
		QVERIFY (f.Edits_.isEmpty ());
	}

	void TagsLineEditTest::testHandEditIsReportedOnce ()
	{
		Fixture f;
		f.Edit_.SetTags ({ A });

		f.Edit_.setText ("a; c"_qs);
		f.FinishEditing ();
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, QStringList { A });
		QCOMPARE (f.Edits_.last ().After_, (QStringList { A, C }));

		f.FinishEditing ();
		QCOMPARE (f.Edits_.size (), 1);

		f.Edit_.setText (A);
		f.FinishEditing ();
		QCOMPARE (f.Edits_.size (), 2);
		QCOMPARE (f.Edits_.last ().Before_, (QStringList { A, C }));
		QCOMPARE (f.Edits_.last ().After_, QStringList { A });
	}

	void TagsLineEditTest::testUpdateTagsKeepsAnEditInProgress ()
	{
		Fixture f;
		f.Edit_.SetTags ({ A });

		f.Edit_.UpdateTags ({ A, Z });
		QCOMPARE (f.Edit_.text (), "a; z"_qs);
		QVERIFY (f.Edits_.isEmpty ());

		f.Edit_.setText ("a; z; c"_qs);
		f.Edit_.UpdateTags ({ A });
		QCOMPARE (f.Edit_.text (), "a; z; c"_qs);

		f.FinishEditing ();
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, (QStringList { A, Z }));
		QCOMPARE (f.Edits_.last ().After_, (QStringList { A, Z, C }));
	}

	void TagsLineEditTest::testCompletionReplacesTheTokenUnderCursor_data ()
	{
		QTest::addColumn<QString> ("separator");
		QTest::addColumn<QString> ("text");
		QTest::addColumn<int> ("cursor");
		QTest::addColumn<QString> ("completion");
		QTest::addColumn<QString> ("expected");
		QTest::addColumn<int> ("expectedCursor");

		const auto standard = "; "_qs;
		const auto music = "music"_qs;
		constexpr int atEnd = -1;

		QTest::newRow ("appending") << standard << "a; b; mu"_qs << atEnd << music << "a; b; music"_qs << 11;
		QTest::newRow ("appending without a space") << standard << "a; b;mu"_qs << atEnd << music << "a; b;music"_qs << 10;
		QTest::newRow ("adjacent separators") << standard << "a;;mu"_qs << atEnd << music << "a;;music"_qs << 8;
		QTest::newRow ("first token") << standard << "mu; b"_qs << 2 << music << "music; b"_qs << 5;
		QTest::newRow ("middle token") << standard << "a; mu; c"_qs << 5 << music << "a; music; c"_qs << 8;
		QTest::newRow ("the only token") << standard << "mu"_qs << atEnd << music << music << 5;
		QTest::newRow ("after a separator") << standard << "a; b; "_qs << atEnd << C << "a; b; c"_qs << 7;
		QTest::newRow ("empty") << standard << QString {} << atEnd << C << C << 1;
		QTest::newRow ("space before the separator") << standard << "mu ; book"_qs << 3 << music << "music; book"_qs << 5;
		QTest::newRow ("space before the separator of a middle token") << standard << "a; b ; c"_qs << 5 << music << "a; music; c"_qs << 8;
		QTest::newRow ("between the separator and its space") << standard << "a; b"_qs << 2 << "bookstore"_qs << "a; bookstore"_qs << 12;
		QTest::newRow ("regex metachar separator") << "|"_qs << "left|mu"_qs << atEnd << music << "left|music"_qs << 10;
		QTest::newRow ("whitespace separator") << " "_qs << "a mu"_qs << atEnd << music << "a music"_qs << 7;
	}

	void TagsLineEditTest::testCompletionReplacesTheTokenUnderCursor ()
	{
		QFETCH (QString, separator);
		QFETCH (QString, text);
		QFETCH (int, cursor);
		QFETCH (QString, completion);
		QFETCH (QString, expected);
		QFETCH (int, expectedCursor);

		Fixture f;
		f.Edit_.SetSeparator (separator);
		f.Edit_.setText (text);
		f.Edit_.setCursorPosition (cursor < 0 ? static_cast<int> (text.size ()) : cursor);

		QSignalSpy chosen { &f.Edit_, &TagsLineEdit::tagsChosen };
		f.Complete (completion);

		QCOMPARE (f.Edit_.text (), expected);
		QCOMPARE (f.Edit_.cursorPosition (), expectedCursor);
		QCOMPARE (chosen.size (), 1);
	}

	void TagsLineEditTest::testSelectorFollowsCompletionModel ()
	{
		Fixture f;
		f.Edit_.SetTags ({ A });

		QSignalSpy chosen { &f.Edit_, &TagsLineEdit::tagsChosen };
		f.TagsModel_.appendRow (new QStandardItem { "d"_qs });
		QCOMPARE (f.Selector ().GetPossibleSelections (), (QStringList { A, B, C, "d"_qs }));
		QCOMPARE (f.Selector ().GetSelections (), QStringList { A });
		QCOMPARE (f.Edit_.text (), A);
		QCOMPARE (chosen.size (), 0);

		f.Edit_.SetTags ({ A, "d"_qs });
		QVERIFY (f.SetChecked (B, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "a; b; d"_qs);
		QCOMPARE (chosen.size (), 1);
	}
}

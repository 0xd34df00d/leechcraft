/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagseditoraddontest.h"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <QAbstractItemView>
#include <QAction>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QLineEdit>
#include <QMenu>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QToolButton>
#include <QTreeView>
#include <QtTest>
#include <util/sll/qtutil.h>
#include "categoryselector.h"
#include "categoryselectorwindow.h"
#include "tagschange.h"
#include "tagseditoraddon.h"

QTEST_MAIN (LC::Util::TagsEditorAddonTest)

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
			QLineEdit Edit_;
			TagsEditorAddon& Addon_ = *new TagsEditorAddon { Edit_, TagsModel_, TagsEditorAddon::Flag::NoSelectorButton };
			CategorySelector Selector_;
			QList<TagsChange> Edits_;

			Fixture ()
			{
				for (const auto& tag : { A, B, C })
					TagsModel_.appendRow (new QStandardItem { tag });

				Addon_.BindSelector (Selector_);
				QObject::connect (&Addon_,
						&TagsEditorAddon::tagsCommitted,
						&Addon_,
						[this] (const TagsChange& change) { Edits_ << change; });
			}

			static bool SetChecked (const CategorySelector& selector, const QString& tag, Qt::CheckState state)
			{
				const auto model = selector.findChild<QTreeView*> ()->model ();
				const auto& found = model->match (model->index (0, 0), Qt::DisplayRole, tag, 1, Qt::MatchExactly);
				return !found.isEmpty () && model->setData (found.front (), state, Qt::CheckStateRole);
			}

			bool SetChecked (const QString& tag, Qt::CheckState state) const
			{
				return SetChecked (Selector_, tag, state);
			}

			void FinishEditing ()
			{
				emit Edit_.editingFinished ();
			}

			QCompleter& Completer () const
			{
				return *Addon_.findChild<QCompleter*> ();
			}

			void Complete (const QString& completion)
			{
				emit Completer ().activated (completion);
			}
		};
	}

	void TagsEditorAddonTest::initMain ()
	{
		qputenv ("QT_QPA_PLATFORM", "offscreen");
	}

	void TagsEditorAddonTest::testSelectorChoiceIsReported ()
	{
		Fixture f;
		f.Addon_.SetTags ({ A, B });

		QSignalSpy picked { &f.Addon_, &TagsEditorAddon::tagPicked };
		QVERIFY (f.SetChecked (B, Qt::Unchecked));

		QCOMPARE (f.Edit_.text (), A);
		QCOMPARE (picked.size (), 1);
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, (QStringList { A, B }));
		QCOMPARE (f.Edits_.last ().After_, QStringList { A });
	}

	void TagsEditorAddonTest::testSelectorFollowsNonCanonicalText ()
	{
		Fixture f;
		f.Edit_.setText ("a;b"_qs);
		QCOMPARE (f.Selector_.GetSelections (), (QStringList { A, B }));

		QVERIFY (f.SetChecked (C, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "a; b; c"_qs);
	}

	void TagsEditorAddonTest::testSelectorChoiceKeepsUnknownTokens ()
	{
		Fixture f;
		f.Edit_.setText ("a; zzz"_qs);
		QCOMPARE (f.Selector_.GetSelections (), QStringList { A });

		QVERIFY (f.SetChecked (B, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "a; b; zzz"_qs);

		QVERIFY (f.SetChecked (A, Qt::Unchecked));
		QCOMPARE (f.Edit_.text (), "b; zzz"_qs);
	}

	void TagsEditorAddonTest::testSetTagsSyncsTheSelectorWithBlockedSignals ()
	{
		Fixture f;
		f.Addon_.SetTags ({ A });
		{
			const QSignalBlocker blocker { &f.Edit_ };
			f.Addon_.SetTags ({ B });
		}
		QCOMPARE (f.Selector_.GetSelections (), QStringList { B });

		QVERIFY (f.SetChecked (C, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "b; c"_qs);
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, QStringList { B });
		QCOMPARE (f.Edits_.last ().After_, (QStringList { B, C }));
	}

	void TagsEditorAddonTest::testNestedSetTagsWins ()
	{
		Fixture f;

		// Like a text change handler answering with tags of its own.
		auto nested = false;
		QObject::connect (&f.Edit_, &QLineEdit::textChanged, &f.Edit_, [&]
				{
					if (!std::exchange (nested, true))
						f.Addon_.SetTags ({ B });
				});
		f.Addon_.SetTags ({ A });
		QCOMPARE (f.Edit_.text (), B);
		QCOMPARE (f.Selector_.GetSelections (), QStringList { B });

		QVERIFY (f.SetChecked (C, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "b; c"_qs);
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, QStringList { B });
	}

	void TagsEditorAddonTest::testUnchangedTagsAreNotReported ()
	{
		Fixture f;
		f.Addon_.SetTags ({ A, B });

		f.FinishEditing ();
		f.Edit_.setText ("b; a"_qs);
		f.FinishEditing ();
		f.Edit_.setText ("a;b"_qs);
		f.FinishEditing ();
		QVERIFY (f.Edits_.isEmpty ());
	}

	void TagsEditorAddonTest::testSetTagsIsNeverReported ()
	{
		Fixture f;
		f.Addon_.SetTags ({ A });

		// Like a text change handler taking the focus away.
		QObject::connect (&f.Edit_, &QLineEdit::textChanged, &f.Edit_, [&f] { f.FinishEditing (); });
		f.Addon_.SetTags ({ B });
		QVERIFY (f.Edits_.isEmpty ());
	}

	void TagsEditorAddonTest::testHandEditIsReportedOnce ()
	{
		Fixture f;
		f.Addon_.SetTags ({ A });

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

	void TagsEditorAddonTest::testUpdateTagsKeepsAnEditInProgress ()
	{
		Fixture f;
		f.Addon_.SetTags ({ A });

		f.Addon_.UpdateTags ({ A, Z });
		QCOMPARE (f.Edit_.text (), "a; z"_qs);
		QVERIFY (f.Edits_.isEmpty ());

		f.Edit_.setText ("a; z; c"_qs);
		f.Addon_.UpdateTags ({ A });
		QCOMPARE (f.Edit_.text (), "a; z; c"_qs);

		f.FinishEditing ();
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().Before_, (QStringList { A, Z }));
		QCOMPARE (f.Edits_.last ().After_, (QStringList { A, Z, C }));
	}

	void TagsEditorAddonTest::testCompletionReplacesTheTokenUnderCursor_data ()
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

	void TagsEditorAddonTest::testCompletionReplacesTheTokenUnderCursor ()
	{
		QFETCH (QString, separator);
		QFETCH (QString, text);
		QFETCH (int, cursor);
		QFETCH (QString, completion);
		QFETCH (QString, expected);
		QFETCH (int, expectedCursor);

		Fixture f;
		f.Addon_.SetSeparator (separator);
		f.Edit_.setText (text);
		f.Edit_.setCursorPosition (cursor < 0 ? static_cast<int> (text.size ()) : cursor);

		QSignalSpy picked { &f.Addon_, &TagsEditorAddon::tagPicked };
		f.Complete (completion);

		QCOMPARE (f.Edit_.text (), expected);
		QCOMPARE (f.Edit_.cursorPosition (), expectedCursor);
		QCOMPARE (picked.size (), 1);
	}

	void TagsEditorAddonTest::testEnterAcceptsTheCompletion ()
	{
		Fixture f;
		f.Edit_.show ();
		QSignalSpy finished { &f.Edit_, &QLineEdit::editingFinished };

		QTest::keyClicks (&f.Edit_, C);
		const auto popup = f.Completer ().popup ();
		QVERIFY (popup->isVisible ());
		QCOMPARE (popup->currentIndex ().data ().toString (), C);

		QTest::keyClick (popup, Qt::Key_Return);
		QVERIFY (!popup->isVisible ());
		QCOMPARE (f.Edit_.text (), C);
		QCOMPARE (finished.size (), 0);
		QCOMPARE (f.Edits_.size (), 1);
		QCOMPARE (f.Edits_.last ().After_, QStringList { C });

		QTest::keyClicks (&f.Edit_, "; b"_qs);
		QVERIFY (popup->isVisible ());
		QTest::keyClick (popup, Qt::Key_Escape);
		QVERIFY (!popup->isVisible ());
		QCOMPARE (f.Edit_.text (), "c; b"_qs);
		QCOMPARE (f.Edits_.size (), 1);
	}

	void TagsEditorAddonTest::testCursorMoveRefreshesCompletion ()
	{
		Fixture f;
		f.Edit_.show ();

		QTest::keyClicks (&f.Edit_, "b; c"_qs);
		const auto popup = f.Completer ().popup ();
		QVERIFY (popup->isVisible ());
		QCOMPARE (popup->currentIndex ().data ().toString (), C);

		QTest::keyClick (&f.Edit_, Qt::Key_Home);
		QVERIFY (popup->isVisible ());
		QCOMPARE (popup->currentIndex ().data ().toString (), B);

		QTest::keyClick (popup, Qt::Key_Return);
		QCOMPARE (f.Edit_.text (), "b; c"_qs);
	}

	void TagsEditorAddonTest::testInnerWhitespaceIsKept ()
	{
		const auto group = "Work  friends"_qs;

		Fixture f;
		f.TagsModel_.appendRow (new QStandardItem { group });
		f.Addon_.SetTags ({ group });
		QCOMPARE (f.Addon_.GetTags (), QStringList { group });
		QCOMPARE (f.Selector_.GetSelections (), QStringList { group });

		QVERIFY (f.SetChecked (group, Qt::Unchecked));
		QVERIFY (f.Edit_.text ().isEmpty ());
		QVERIFY (f.SetChecked (group, Qt::Checked));
		QCOMPARE (f.Edit_.text (), group);
		QCOMPARE (f.Edits_.last ().After_, QStringList { group });
	}

	void TagsEditorAddonTest::testSelectorFollowsTheTagsModel ()
	{
		Fixture f;
		f.Addon_.SetTags ({ A, "d"_qs });
		QCOMPARE (f.Selector_.GetSelections (), QStringList { A });

		QSignalSpy picked { &f.Addon_, &TagsEditorAddon::tagPicked };
		f.TagsModel_.appendRow (new QStandardItem { "d"_qs });
		QCOMPARE (f.Selector_.GetSelections (), (QStringList { A, "d"_qs }));
		QCOMPARE (f.Edit_.text (), "a; d"_qs);
		QCOMPARE (picked.size (), 0);

		QVERIFY (f.SetChecked (B, Qt::Checked));
		QCOMPARE (f.Edit_.text (), "a; b; d"_qs);
		QCOMPARE (picked.size (), 1);
	}

	void TagsEditorAddonTest::testSelectorButtonBindsAPopup ()
	{
		Fixture f;
		QVERIFY (!f.Edit_.findChild<QToolButton*> ());

		QLineEdit edit;
		auto& addon = *new TagsEditorAddon { edit, f.TagsModel_ };
		addon.SetTags ({ A });

		const auto popup = edit.findChild<CategorySelectorWindow*> ();
		QVERIFY (popup);
		QVERIFY (!popup->isVisible ());
		QCOMPARE (popup->Selector ().GetSelections (), QStringList { A });

		edit.findChild<QToolButton*> ()->click ();
		QVERIFY (popup->isVisible ());

		QVERIFY (Fixture::SetChecked (popup->Selector (), C, Qt::Checked));
		QCOMPARE (edit.text (), "a; c"_qs);
	}

	void TagsEditorAddonTest::testOneLiveSelectorAtATime ()
	{
		Fixture f;
		CategorySelector other;
		QVERIFY_THROWS_EXCEPTION (std::logic_error, f.Addon_.BindSelector (other));
		QVERIFY_THROWS_EXCEPTION (std::logic_error, f.Addon_.AddSelectorButton ());

		QLineEdit edit;
		auto& addon = *new TagsEditorAddon { edit, f.TagsModel_ };
		QVERIFY_THROWS_EXCEPTION (std::logic_error, addon.BindSelector (other));
		QVERIFY_THROWS_EXCEPTION (std::logic_error, addon.AddSelectorButton ());

		delete edit.findChild<CategorySelectorWindow*> ();
		addon.BindSelector (other);
		QVERIFY (Fixture::SetChecked (other, A, Qt::Checked));
		QCOMPARE (edit.text (), A);
	}

	void TagsEditorAddonTest::testContextMenuOffersTheSelector ()
	{
		Fixture f;
		QLineEdit edit;
		new TagsEditorAddon { edit, f.TagsModel_ };
		edit.show ();

		QContextMenuEvent event { QContextMenuEvent::Mouse, { 1, 1 }, edit.mapToGlobal (QPoint { 1, 1 }) };
		QApplication::sendEvent (&edit, &event);

		const auto menu = edit.findChild<QMenu*> (Qt::FindDirectChildrenOnly);
		QVERIFY (menu);
		QVERIFY (menu->isVisible ());
		const auto& actions = menu->actions ();
		QCOMPARE (actions.at (0)->text (), "Select tags..."_qs);
		QVERIFY (actions.at (1)->isSeparator ());
		QVERIFY (std::ranges::any_of (actions, [] (const QAction *action) { return action->text ().startsWith ("Select All"_qs); }));

		const auto popup = edit.findChild<CategorySelectorWindow*> ();
		QVERIFY (!popup->isVisible ());
		actions.at (0)->trigger ();
		QVERIFY (popup->isVisible ());
		menu->close ();
	}

	void TagsEditorAddonTest::testContextMenuIsStandardWithoutAPopup ()
	{
		Fixture f;
		f.Edit_.show ();

		QContextMenuEvent event { QContextMenuEvent::Mouse, { 1, 1 }, f.Edit_.mapToGlobal (QPoint { 1, 1 }) };
		QApplication::sendEvent (&f.Edit_, &event);

		const auto menu = f.Edit_.findChild<QMenu*> (Qt::FindDirectChildrenOnly);
		QVERIFY (menu);
		QVERIFY (std::ranges::none_of (menu->actions (), [] (const QAction *action) { return action->text () == "Select tags..."_qs; }));
		menu->close ();
	}

	void TagsEditorAddonTest::testAltDownOpensTheSelector ()
	{
		Fixture f;
		QLineEdit edit;
		new TagsEditorAddon { edit, f.TagsModel_ };
		edit.show ();
		QVERIFY (QTest::qWaitForWindowActive (&edit));
		edit.setFocus ();

		const auto popup = edit.findChild<CategorySelectorWindow*> ();
		QVERIFY (!popup->isVisible ());
		QTest::keyClick (&edit, Qt::Key_Down, Qt::AltModifier);
		QVERIFY (popup->isVisible ());
	}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagseditoraddon.h"
#include <stdexcept>
#include <QAbstractItemView>
#include <QAction>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QCursor>
#include <QIcon>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QSet>
#include <QToolButton>
#include <interfaces/core/itagsmanager.h>
#include "gui/lineeditbuttonmanager.h"
#include "categoryselector.h"
#include "categoryselectorwindow.h"
#include "tagschange.h"
#include "util.h"

namespace LC::Util
{
	TagsEditorAddon::TagsEditorAddon (QLineEdit& edit, QAbstractItemModel& model, Flags flags)
	: QObject { &edit }
	, Edit_ { edit }
	, Completer_ { *new QCompleter { &model, this } }
	, Separator_ { GetDefaultTagsSeparator () }
	{
		Completer_.setCompletionRole (Qt::DisplayRole);
		Completer_.setCompletionMode (QCompleter::PopupCompletion);
		Completer_.setWidget (&Edit_);

		connect (&Completer_,
				qOverload<const QString&> (&QCompleter::activated),
				this,
				&TagsEditorAddon::InsertTag);
		connect (&Edit_,
				&QLineEdit::textEdited,
				this,
				&TagsEditorAddon::Complete);
		connect (&Edit_,
				&QLineEdit::cursorPositionChanged,
				this,
				[this]
				{
					if (Completer_.popup ()->isVisible () && PrefixUnderCursor () != CompletionPrefix_)
						Complete ();
				});
		connect (&Edit_,
				&QLineEdit::textChanged,
				this,
				[this]
				{
					if (Selector_)
						Selector_->SetSelections (GetTags ());
				});
		connect (&Edit_,
				&QLineEdit::editingFinished,
				this,
				&TagsEditorAddon::ReportEdit);
		connect (this,
				&TagsEditorAddon::tagPicked,
				this,
				&TagsEditorAddon::ReportEdit);

		if (!(flags & Flag::NoSelectorButton))
			AddSelectorButton ();
	}

	TagsEditorAddon::TagsEditorAddon (QLineEdit& edit, ITagsManager& tm, Flags flags)
	: TagsEditorAddon { edit, *tm.GetModel (), flags }
	{
	}

	QString TagsEditorAddon::GetSeparator () const
	{
		return Separator_;
	}

	void TagsEditorAddon::SetSeparator (const QString& separator)
	{
		if (separator.isEmpty ())
			throw std::invalid_argument { "TagsEditorAddon: the tags separator must not be empty" };

		Separator_ = separator;
	}

	void TagsEditorAddon::AddSelectorButton (LineEditButtonManager *manager)
	{
		const auto popup = new CategorySelectorWindow { CategorySelectorWindow::Buttons::Close, &Edit_ };
		BindSelector (popup->Selector ());

		const auto action = new QAction { QIcon::fromTheme (QStringLiteral ("mail-tagged")), tr ("Select tags..."), popup };
		action->setShortcut (Qt::ALT | Qt::Key_Down);
		action->setShortcutContext (Qt::WidgetShortcut);
		connect (action,
				&QAction::triggered,
				popup,
				[popup]
				{
					popup->move (QCursor::pos ());
					popup->show ();
				});
		Edit_.addAction (action);
		Edit_.installEventFilter (this);
		SelectorAction_ = action;

		if (!manager)
			manager = new LineEditButtonManager { &Edit_ };

		const auto button = new QToolButton { &Edit_ };
		button->setDefaultAction (action);
		button->setIconSize ({ 16, 16 });
		button->setCursor (Qt::ArrowCursor);
		button->setStyleSheet (QStringLiteral ("QToolButton { border: none; padding: 0px; }"));
		manager->Add (button);
	}

	void TagsEditorAddon::BindSelector (CategorySelector& selector)
	{
		if (Selector_)
			throw std::logic_error { "TagsEditorAddon: a selector is already bound" };

		Selector_ = &selector;
		selector.SetModel (*Completer_.model ());
		selector.SetSelections (GetTags ());

		connect (&selector,
				&CategorySelector::selectionEdited,
				this,
				&TagsEditorAddon::ApplyChoice);
	}

	void TagsEditorAddon::SetTags (const QStringList& tags)
	{
		ShownTags_ = tags;
		if (Selector_)
			Selector_->SetSelections (tags);
		Edit_.setText (tags.join (Separator_));
	}

	void TagsEditorAddon::UpdateTags (const QStringList& tags)
	{
		if (IsEdited () || TagsChange { .Before_ = ShownTags_, .After_ = tags }.IsEmpty ())
			return;

		SetTags (tags);
	}

	QStringList TagsEditorAddon::GetTags () const
	{
		return SplitTags (Edit_.text (), Separator_);
	}

	bool TagsEditorAddon::eventFilter (QObject *obj, QEvent *event)
	{
		if (obj == &Edit_)
		{
			if (event->type () != QEvent::ContextMenu || !SelectorAction_)
				return false;

			ShowContextMenu (static_cast<QContextMenuEvent*> (event)->globalPos ());
			return true;
		}

		if (event->type () != QEvent::KeyPress || obj != Completer_.popup ())
			return false;

		const auto popup = Completer_.popup ();
		switch (static_cast<QKeyEvent*> (event)->key ())
		{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		case Qt::Key_Tab:
		{
			const auto& current = popup->currentIndex ();
			popup->hide ();
			if (current.isValid ())
				InsertTag (current.data (Completer_.completionRole ()).toString ());
			return true;
		}
		case Qt::Key_Backtab:
			popup->hide ();
			return true;
		default:
			return false;
		}
	}

	void TagsEditorAddon::ShowContextMenu (const QPoint& pos)
	{
		const auto menu = Edit_.createStandardContextMenu ();
		menu->setAttribute (Qt::WA_DeleteOnClose);

		const auto first = menu->actions ().value (0);
		menu->insertAction (first, SelectorAction_);
		menu->insertSeparator (first);
		menu->popup (pos);
	}

	TagsEditorAddon::TokenRange TagsEditorAddon::TokenUnderCursor () const
	{
		const auto& text = Edit_.text ();
		const auto cursor = Edit_.cursorPosition ();
		const auto& delimiter = GetTagsDelimiter (Separator_);

		qsizetype begin = 0;
		auto end = text.size ();
		for (auto pos = text.indexOf (delimiter); pos != -1; pos = text.indexOf (delimiter, pos + delimiter.size ()))
		{
			if (pos >= cursor)
			{
				end = pos;
				break;
			}
			begin = pos + delimiter.size ();
		}

		// The whitespace after a separator belongs to it, the whitespace before the next one to the token.
		while (begin < end && text.at (begin).isSpace ())
			++begin;

		return { .Begin_ = begin, .End_ = end };
	}

	QString TagsEditorAddon::PrefixUnderCursor () const
	{
		const auto [begin, end] = TokenUnderCursor ();
		return Edit_.text ().mid (begin, end - begin).trimmed ();
	}

	void TagsEditorAddon::Complete ()
	{
		CompletionPrefix_ = PrefixUnderCursor ();
		Completer_.setCompletionPrefix (CompletionPrefix_);

		// The completer hands the keys to the edit by a direct event() call, so the edit's
		// filters never see them; filtering the popup instead keeps Enter from finishing the edit.
		const auto popup = Completer_.popup ();
		popup->installEventFilter (this);
		popup->setCurrentIndex (Completer_.completionModel ()->index (0, 0));
		Completer_.complete ();
	}

	void TagsEditorAddon::InsertTag (const QString& completion)
	{
		const auto [begin, end] = TokenUnderCursor ();
		auto text = Edit_.text ();
		text.replace (begin, end - begin, completion);
		Edit_.setText (text);
		Edit_.setCursorPosition (static_cast<int> (begin + completion.size ()));

		emit tagPicked ();
	}

	void TagsEditorAddon::ApplyChoice (const QStringList& chosen)
	{
		const auto& model = *Completer_.model ();
		QSet<QString> known;
		for (int row = 0; row < model.rowCount (); ++row)
			known << model.index (row, 0).data ().toString ();

		auto tags = chosen;
		for (const auto& tag : GetTags ())
			if (!known.contains (tag) && !tags.contains (tag))
				tags << tag;

		Edit_.setText (tags.join (Separator_));
		emit tagPicked ();
	}

	bool TagsEditorAddon::IsEdited () const
	{
		return !TagsChange { .Before_ = ShownTags_, .After_ = GetTags () }.IsEmpty ();
	}

	void TagsEditorAddon::ReportEdit ()
	{
		const TagsChange change { .Before_ = ShownTags_, .After_ = GetTags () };
		if (change.IsEmpty ())
			return;

		ShownTags_ = change.After_;
		emit tagsCommitted (change);
	}
}

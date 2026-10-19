/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagslineedit.h"
#include <QtDebug>
#include <QTimer>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QAbstractItemView>
#include "gui/lineeditbuttonmanager.h"
#include "tagschange.h"
#include "tagscompletionmodel.h"
#include "tagscompleter.h"
#include "util.h"

namespace LC::Util
{
	TagsLineEdit::TagsLineEdit (QWidget *parent)
	: QLineEdit (parent)
	, Separator_ (GetDefaultTagsSeparator ())
	{
		connect (this,
				&QLineEdit::editingFinished,
				this,
				&TagsLineEdit::ReportEdit);
		connect (this,
				&TagsLineEdit::tagsChosen,
				this,
				&TagsLineEdit::ReportEdit);
	}

	void TagsLineEdit::AddSelector (LineEditButtonManager *mgr)
	{
		auto selector = new CategorySelector (this);
		selector->hide ();

		AddSelector (selector);

		if (!mgr)
			mgr = new LineEditButtonManager { this };

		auto button = new QToolButton { this };
		button->setIconSize ({ 16, 16 });
		button->setIcon (QIcon::fromTheme (QStringLiteral ("mail-tagged")));
		button->setCursor (Qt::ArrowCursor);
		button->setStyleSheet (QStringLiteral ("QToolButton { border: none; padding: 0px; }"));

		mgr->Add (button);

		connect (button,
				&QToolButton::clicked,
				this,
				[selector]
				{
					selector->move (QCursor::pos ());
					selector->show ();
				});
	}

	void TagsLineEdit::AddSelector (CategorySelector *selector)
	{
		CategorySelector_ = selector;
		CategorySelector_->SetSeparator (Separator_);

		const auto model = Completer_->model ();
		const auto syncPossibleSelections = [this, model]
		{
			QStringList tags;
			tags.reserve (model->rowCount ());
			for (int i = 0; i < model->rowCount (); ++i)
				tags << model->data (model->index (i, 0)).toString ();

			CategorySelector_->SetPossibleSelections (tags);
			CategorySelector_->SetSelectionsFromString (text ());
		};
		syncPossibleSelections ();

		const auto syncOn = [&] (auto signal) { connect (model, signal, this, syncPossibleSelections); };
		syncOn (&QAbstractItemModel::rowsInserted);
		syncOn (&QAbstractItemModel::rowsRemoved);
		syncOn (&QAbstractItemModel::dataChanged);
		syncOn (&QAbstractItemModel::modelReset);

		connect (CategorySelector_,
				&CategorySelector::tagsSelectionChanged,
				this,
				[this] (const QStringList& tags)
				{
					setText (tags.join (Separator_));
					emit tagsChosen ();
				});

		connect (this,
				&QLineEdit::textChanged,
				CategorySelector_,
				&CategorySelector::SetSelectionsFromString);
	}

	QString TagsLineEdit::GetSeparator () const
	{
		return Separator_;
	}

	void TagsLineEdit::SetSeparator (const QString& sep)
	{
		if (sep.isEmpty ())
			qFatal ("TagsLineEdit: the tags separator must not be empty");

		Separator_ = sep;
		if (CategorySelector_)
			CategorySelector_->SetSeparator (sep);
	}

	void TagsLineEdit::InsertTag (const QString& completion)
	{
		const auto [begin, end] = TokenUnderCursor ();
		auto wtext = text ();
		wtext.replace (begin, end - begin, completion);
		setText (wtext);
		setCursorPosition (static_cast<int> (begin + completion.size ()));

		emit tagsChosen ();
	}

	void TagsLineEdit::SetTags (const QStringList& tags)
	{
		ShownTags_ = tags;
		if (CategorySelector_)
			CategorySelector_->SetSelections (tags);
		setText (tags.join (Separator_));
	}

	void TagsLineEdit::UpdateTags (const QStringList& tags)
	{
		if (IsEdited () || TagsChange { .Before_ = ShownTags_, .After_ = tags }.IsEmpty ())
			return;

		SetTags (tags);
	}

	QStringList TagsLineEdit::GetTags () const
	{
		return SplitTags (text (), Separator_);
	}

	bool TagsLineEdit::IsEdited () const
	{
		return !TagsChange { .Before_ = ShownTags_, .After_ = GetTags () }.IsEmpty ();
	}

	void TagsLineEdit::ReportEdit ()
	{
		const TagsChange change { .Before_ = ShownTags_, .After_ = GetTags () };
		if (change.IsEmpty ())
			return;

		ShownTags_ = change.After_;
		emit tagsEdited (change);
	}

	void TagsLineEdit::keyPressEvent (QKeyEvent *e)
	{
		if (Completer_ && Completer_->popup ()->isVisible ())
			switch (e->key ())
			{
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
			case Qt::Key_Backtab:
				e->ignore ();
				return;
			default:
				break;
			}

		QLineEdit::keyPressEvent (e);

		bool cos = e->modifiers () & (Qt::ControlModifier |
				Qt::ShiftModifier |
				Qt::AltModifier |
				Qt::MetaModifier);
		bool isShortcut = e->modifiers () & (Qt::ControlModifier |
				Qt::AltModifier |
				Qt::ShiftModifier);
		if (!Completer_ ||
				(cos && e->text ().isEmpty ()) ||
				isShortcut)
			return;

		QString completionPrefix = TextUnderCursor ();
		Completer_->setCompletionPrefix (completionPrefix);
		Completer_->popup ()->
			setCurrentIndex (Completer_->completionModel ()->index (0, 0));
		Completer_->complete ();
	}

	void TagsLineEdit::focusInEvent (QFocusEvent *e)
	{
		if (Completer_)
			Completer_->setWidget (this);
		QLineEdit::focusInEvent (e);
	}

	void TagsLineEdit::contextMenuEvent (QContextMenuEvent *e)
	{
		if (!CategorySelector_ || CategorySelector_->parentWidget () != this)
		{
			QLineEdit::contextMenuEvent (e);
			return;
		}

		CategorySelector_->move (e->globalPos ());
		CategorySelector_->show ();
	}

	void TagsLineEdit::SetCompleter (TagsCompleter *c)
	{
		if (Completer_)
			disconnect (Completer_,
					nullptr,
					this,
					nullptr);

		Completer_ = c;

		if (!Completer_)
			return;

		Completer_->setWidget (this);
		Completer_->setCompletionMode (QCompleter::PopupCompletion);
		connect (Completer_,
				qOverload<const QString&> (&QCompleter::activated),
				this,
				&TagsLineEdit::InsertTag);
	}

	TagsLineEdit::TokenRange TagsLineEdit::TokenUnderCursor () const
	{
		const auto& wtext = text ();
		const auto cursor = cursorPosition ();
		const auto& delimiter = GetTagsDelimiter (Separator_);

		qsizetype begin = 0;
		auto end = wtext.size ();
		for (auto pos = wtext.indexOf (delimiter); pos != -1; pos = wtext.indexOf (delimiter, pos + delimiter.size ()))
		{
			if (pos >= cursor)
			{
				end = pos;
				break;
			}
			begin = pos + delimiter.size ();
		}

		// The whitespace after a separator belongs to it, the whitespace before the next one to the token.
		while (begin < end && wtext.at (begin).isSpace ())
			++begin;

		return { .Begin_ = begin, .End_ = end };
	}

	QString TagsLineEdit::TextUnderCursor () const
	{
		const auto [begin, end] = TokenUnderCursor ();
		return text ().mid (begin, end - begin);
	}
}

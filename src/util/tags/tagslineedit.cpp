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
#include <util/sll/qtutil.h>
#include "gui/lineeditbuttonmanager.h"
#include "tagscompletionmodel.h"
#include "tagscompleter.h"
#include "util.h"

namespace LC::Util
{
	TagsLineEdit::TagsLineEdit (QWidget *parent)
	: QLineEdit (parent)
	, Separator_ (GetDefaultTagsSeparator ())
	{
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

		QAbstractItemModel *model = Completer_->model ();

		QStringList initialTags;
		for (int i = 0; i < model->rowCount (); ++i)
			initialTags << model->data (model->index (i, 0)).toString ();
		CategorySelector_->SetPossibleSelections (initialTags);

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
		Separator_ = sep;
		if (CategorySelector_)
			CategorySelector_->SetSeparator (sep);
	}

	void TagsLineEdit::InsertTag (const QString& completion)
	{
		if (Completer_->widget () != this)
			return;

		QString wtext = text ();
		if (completion.startsWith (wtext))
			wtext.clear ();
		int pos = wtext.lastIndexOf (Separator_);
		if (pos >= 0)
			wtext = wtext.left (pos).append (Separator_);
		else
			wtext.clear ();
		wtext.append (completion);
		wtext = wtext.simplified ();
		setText (wtext);

		emit tagsChosen ();
	}

	void TagsLineEdit::setTags (const QStringList& tags)
	{
		setText (tags.join (Separator_));
		if (CategorySelector_)
			CategorySelector_->SetSelections (tags);
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

		QString completionPrefix = textUnderCursor ();
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

	QString TagsLineEdit::textUnderCursor () const
	{
		auto rxStr = Separator_;
		rxStr.replace (' ', R"(\s*)"_ql);

		QRegularExpression rx { rxStr };

		QString wtext = text ();
		int pos = cursorPosition () - 1;
		int last = wtext.indexOf (rx, pos);
		int first = wtext.lastIndexOf (rx, pos);
		if (first == -1)
			first = 0;
		if (last == -1)
			last = wtext.size ();
		return wtext.mid (first, last - first);
	}
}

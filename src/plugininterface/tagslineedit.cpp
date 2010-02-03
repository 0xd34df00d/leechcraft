/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "tagslineedit.h"
#include <QtDebug>
#include <QTimer>
#include <QCompleter>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include "tagscompletionmodel.h"
#include "tagscompleter.h"

using namespace LeechCraft::Util;

TagsLineEdit::TagsLineEdit (QWidget *parent)
: QLineEdit (parent)
, Completer_ (0)
{

}

void TagsLineEdit::AddSelector ()
{
	CategorySelector_.reset (new CategorySelector (parentWidget ()));
	CategorySelector_->hide ();

	QAbstractItemModel *model = Completer_->model ();

	connect (model,
			SIGNAL (tagsUpdated (const QStringList&)),
			this,
			SLOT (handleTagsUpdated (const QStringList&)));

	QStringList initialTags;
	for (int i = 0; i < model->rowCount (); ++i)
		initialTags << model->data (model->index (i, 0)).toString ();
	handleTagsUpdated (initialTags);

	connect (CategorySelector_.get (),
			SIGNAL (selectionChanged (const QStringList&)),
			this,
			SLOT (handleSelectionChanged (const QStringList&)));

	connect (this,
			SIGNAL (textChanged (const QString&)),
			CategorySelector_.get (),
			SLOT (lineTextChanged (const QString&)));
}

void TagsLineEdit::insertTag (const QString& completion)
{
	if (Completer_->widget () != this)
		return;

	QString wtext = text ();
	if (completion.startsWith (wtext))
		wtext.clear ();
	int pos = wtext.lastIndexOf ("; ");
	if (pos >= 0)
		wtext = wtext.left (pos).append ("; ");
	else
		wtext.clear ();
	wtext.append (completion);
	wtext = wtext.simplified ();
	setText (wtext);

	emit tagsChosen ();
}

void TagsLineEdit::handleTagsUpdated (const QStringList& tags)
{
	disconnect (CategorySelector_.get (),
			SIGNAL (selectionChanged (const QStringList&)),
			this,
			SLOT (handleSelectionChanged (const QStringList&)));
	CategorySelector_->SetPossibleSelections (tags);
	connect (CategorySelector_.get (),
			SIGNAL (selectionChanged (const QStringList&)),
			this,
			SLOT (handleSelectionChanged (const QStringList&)));
}

void TagsLineEdit::handleSelectionChanged (const QStringList& tags)
{
	setText (tags.join ("; "));

	emit tagsChosen ();
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
	if (!CategorySelector_.get ())
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
				0,
				this,
				0);

	Completer_ = c;

	if (!Completer_)
		return;

	Completer_->setWidget (this);
	Completer_->setCompletionMode (QCompleter::PopupCompletion);
	connect (Completer_,
			SIGNAL (activated (const QString&)),
			this,
			SLOT (insertTag (const QString&)));
}

QString TagsLineEdit::textUnderCursor () const
{
	QRegExp rx (";\\s*");
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


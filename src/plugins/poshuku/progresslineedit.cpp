/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "progresslineedit.h"
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>
#include <QToolBar>
#include <QToolButton>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "urlcompletionmodel.h"
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
	ProgressLineEdit::ProgressLineEdit (QWidget *parent)
	: QLineEdit (parent)
	, IsCompleting_ (false)
	{
		QCompleter *completer = new QCompleter (this);
		completer->setModel (Core::Instance ().GetURLCompletionModel ());
		completer->setCompletionRole (URLCompletionModel::RoleURL);
		completer->setCompletionMode (QCompleter::UnfilteredPopupCompletion);
		setCompleter (completer);

		ClearButton_ = new QToolButton (this);
		ClearButton_->setIcon (Core::Instance ().GetProxy ()->GetIcon ("clearall"));
		ClearButton_->setCursor (Qt::PointingHandCursor);
		ClearButton_->setStyleSheet ("QToolButton { border: none; padding: 0px; }");
		ClearButton_->hide ();

		int frameWidth = style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
		setStyleSheet (QString ("QLineEdit { padding-right: %1px; } ")
				.arg (ClearButton_->sizeHint ().width () + frameWidth + 1));

		connect (ClearButton_,
				SIGNAL (clicked ()),
				this,
				SLOT (clear ()));

		connect (completer,
				SIGNAL (activated (const QModelIndex&)),
				this,
				SLOT (handleCompleterActivated ()));

		connect (this,
				SIGNAL (textEdited (const QString&)),
				Core::Instance ().GetURLCompletionModel (),
				SLOT (setBase (const QString&)));

		connect (this,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (textChanged (const QString&)));
	}

	ProgressLineEdit::~ProgressLineEdit ()
	{
	}

	bool ProgressLineEdit::IsCompleting () const
	{
		return IsCompleting_;
	}

	QObject* ProgressLineEdit::GetObject ()
	{
		return this;
	}

	int ProgressLineEdit::ButtonsCount () const
	{
		return VisibleButtons_.count ();
	}

	QToolButton* ProgressLineEdit::AddAction (QAction *action, bool hideOnEmtpyUrl)
	{
		return InsertAction (action, 1, hideOnEmtpyUrl);
	}

	QToolButton* ProgressLineEdit::InsertAction (QAction *action, int id, bool hideOnEmtpyUrl)
	{
		if (Action2Button_.contains (action))
			return Action2Button_ [action];

		QToolButton *button = new QToolButton (this);
		button->setCursor (Qt::PointingHandCursor);
		button->setDefaultAction (action);
		button->setStyleSheet ("QToolButton {border: none; padding: 0px;}");

		connect (button,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleTriggeredButton (QAction*)));

		button->hide ();

		if (hideOnEmtpyUrl)
			HideButtons_ << button;

		Action2Button_ [action] = button;

		VisibleButtons_.insert (id == -1 ? ButtonsCount () - 1 : id, button);

		const QSize& msz = minimumSizeHint ();
		setMinimumSize (qMax (msz.width (), button->sizeHint ().height () + 2),
				qMax (msz.height (), button->sizeHint ().height () + 2));

		RepaintButtons ();

		return button;
	}

	QToolButton* ProgressLineEdit::GetButtonFromAction (QAction *action) const
	{
		if (Action2Button_.contains (action))
			return Action2Button_ [action];

		return 0;
	}

	void ProgressLineEdit::RemoveAction (QAction *action)
	{
		if (!Action2Button_.contains (action))
			return;

		QToolButton *btn = Action2Button_.take (action);
		VisibleButtons_.removeAll (btn);
		HideButtons_.removeAll (btn);
		btn->deleteLater ();
		RepaintButtons ();
	}

	void ProgressLineEdit::SetVisible (QAction *action, bool visible)
	{
		if (!Action2Button_.contains (action))
			return;

		Action2Button_ [action]->setVisible (visible);
		RepaintButtons ();
	}

	void ProgressLineEdit::handleCompleterActivated ()
	{
		PreviousUrl_ = text ();
		emit returnPressed ();
	}

	void ProgressLineEdit::keyPressEvent (QKeyEvent *event)
	{
		switch (event->key ())
		{
		case Qt::Key_Escape:
			setText (PreviousUrl_);
			break;
		case Qt::Key_Return:
		case Qt::Key_Enter:
			PreviousUrl_ = text ();
		default:
			QLineEdit::keyPressEvent (event);
		}
	}

	void ProgressLineEdit::resizeEvent (QResizeEvent*)
	{
		RepaintButtons ();
	}

	void ProgressLineEdit::textChanged (const QString& text)
	{
		if (text.isEmpty ())
		{
			VisibleButtons_.removeAll (ClearButton_);
			ClearButton_->hide ();
			Q_FOREACH (QToolButton *btn, HideButtons_)
				btn->hide ();
		}
		else if (!VisibleButtons_.contains (ClearButton_))
		{
			VisibleButtons_.push_back (ClearButton_);
			ClearButton_->show ();
			Q_FOREACH (QToolButton *btn, HideButtons_)
				btn->show ();
		}

		RepaintButtons ();

		if (!text.isEmpty () && PreviousUrl_.isEmpty ())
			PreviousUrl_ = text;
	}

	void ProgressLineEdit::RepaintButtons ()
	{
		const int frameWidth = style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
		int rightBorder = 0;
		for (int i = VisibleButtons_.count () - 1; i >= 0; --i)
		{
			QToolButton *btn = VisibleButtons_ [i];
			const QSize& bmSz = btn->sizeHint ();
			rightBorder += bmSz.width ();
			btn->move (rect ().right () - frameWidth - rightBorder,
					   (rect ().bottom () + 1 - bmSz.height ()) / 2);
		}
	}

	void ProgressLineEdit::handleTriggeredButton (QAction *action)
	{
		emit actionTriggered (action, text ());
	}
}
}

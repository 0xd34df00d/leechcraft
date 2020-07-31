/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "progresslineedit.h"
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>
#include <QToolBar>
#include <QToolButton>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "urlcompletionmodel.h"
#include "core.h"

namespace LC
{
namespace Poshuku
{
	ProgressLineEdit::ProgressLineEdit (QWidget *parent)
	: QLineEdit (parent)
	, IsCompleting_ (false)
	{
		setPlaceholderText ("about:blank");

		QCompleter *completer = new QCompleter (this);
		completer->setModel (Core::Instance ().GetURLCompletionModel ());
		completer->setCompletionRole (URLCompletionModel::RoleURL);
		completer->setCompletionMode (QCompleter::UnfilteredPopupCompletion);
		completer->setMaxVisibleItems (15);
		setCompleter (completer);

		ClearButton_ = new QToolButton (this);
		ClearButton_->setIcon (Core::Instance ().GetProxy ()->
				GetIconThemeManager ()->GetIcon ("edit-clear-locationbar-ltr"));
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
				SLOT (setBase (const QString&)),
				Qt::QueuedConnection);

		connect (this,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (textChanged (const QString&)));
	}

	bool ProgressLineEdit::IsCompleting () const
	{
		return IsCompleting_;
	}

	QObject* ProgressLineEdit::GetQObject ()
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
			[[fallthrough]];
		default:
			QLineEdit::keyPressEvent (event);
		}
	}

	void ProgressLineEdit::resizeEvent (QResizeEvent*)
	{
		RepaintButtons ();
	}

	void ProgressLineEdit::contextMenuEvent (QContextMenuEvent *e)
	{
		QString cbText = qApp->clipboard ()->text (QClipboard::Clipboard);
		if (cbText.isEmpty ())
			cbText = qApp->clipboard ()->text (QClipboard::Selection);
		if (cbText.isEmpty ())
		{
			QLineEdit::contextMenuEvent (e);
			return;
		}

		QMenu *menu = createStandardContextMenu ();
		const auto& acts = menu->actions ();
		QAction *before = 0;
		for (int i = 0; i < acts.size (); ++i)
			if (acts.at (i)->shortcut () == QKeySequence (QKeySequence::Paste))
			{
				before = acts.value (i + 1);
				break;
			}

		QAction *pasteGo = new QAction (tr ("Paste and go"), menu);
		pasteGo->setData (cbText);
		connect (pasteGo,
				SIGNAL (triggered ()),
				this,
				SLOT (pasteGo ()));
		if (before)
			menu->insertAction (before, pasteGo);
		else
		{
			menu->addSeparator ();
			menu->addAction (pasteGo);
		}

		menu->exec (e->globalPos ());

		menu->deleteLater ();
	}

	void ProgressLineEdit::textChanged (const QString& text)
	{
		if (text.isEmpty ())
		{
			VisibleButtons_.removeAll (ClearButton_);
			ClearButton_->hide ();
			for (auto btn : HideButtons_)
				btn->hide ();
		}
		else if (!VisibleButtons_.contains (ClearButton_))
		{
			VisibleButtons_.push_back (ClearButton_);
			ClearButton_->show ();
			for (auto btn : HideButtons_)
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
		int realBorder = 0;
		for (int i = VisibleButtons_.count () - 1; i >= 0; --i)
		{
			QToolButton *btn = VisibleButtons_ [i];
			const QSize& bmSz = btn->sizeHint ();
			rightBorder += bmSz.width ();
			if (i > 0)
				realBorder += bmSz.width ();

			btn->move (rect ().right () - frameWidth - rightBorder,
					(rect ().bottom () + 1 - bmSz.height ()) / 2);
		}

		const QMargins& margins = textMargins ();
		setTextMargins (margins.left (),
				margins.top (),
				realBorder + frameWidth,
				margins.bottom ());
	}

	void ProgressLineEdit::handleTriggeredButton (QAction *action)
	{
		emit actionTriggered (action, text ());
	}

	void ProgressLineEdit::pasteGo ()
	{
		QAction *act = qobject_cast<QAction*> (sender ());
		const QString& text = act->data ().toString ();
		setText (text);
		emit returnPressed ();
	}
}
}

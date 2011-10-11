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
		QSize msz = minimumSizeHint ();
		setMinimumSize (qMax (msz.width (),
					ClearButton_->sizeHint ().height () + frameWidth * 2 + 2),
				qMax (msz.height (),
					ClearButton_->sizeHint ().height () + frameWidth * 2 + 2));

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
		QSize sz = ClearButton_->sizeHint ();
		int frameWidth = style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
		ClearButton_->move (rect ().right () - frameWidth - sz.width (),
				(rect ().bottom () + 1 - sz.height ())/2);
	}

	void ProgressLineEdit::textChanged (const QString& text)
	{
		ClearButton_->setVisible (!text.isEmpty ());

		if (!text.isEmpty () && PreviousUrl_.isEmpty ())
			PreviousUrl_ = text;
	}
}
}

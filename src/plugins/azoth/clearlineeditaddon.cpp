/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "clearlineeditaddon.h"
#include <QLineEdit>
#include <QToolButton>
#include <QApplication>
#include <QStyle>

namespace LeechCraft
{
namespace Azoth
{
	ClearLineEditAddon::ClearLineEditAddon (ICoreProxy_ptr proxy, QLineEdit *edit)
	: QObject (edit)
	, Button_ (new QToolButton (edit))
	, Edit_ (edit)
	{
		const bool isRtl = QApplication::layoutDirection () == Qt::RightToLeft;
		const auto& icon = proxy->GetIcon (isRtl ?
				"edit-clear-locationbar-ltr" :
				"edit-clear-locationbar-rtl");

		Button_->setIconSize (QSize (16, 16));
		Button_->setIcon (icon);
		Button_->setCursor (Qt::ArrowCursor);
		Button_->setStyleSheet ("QToolButton { border: none; padding: 0px; }");
		Button_->hide ();

		connect (Button_,
				SIGNAL (clicked ()),
				edit,
				SLOT (clear ()));
		connect (edit,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (updateButton (QString)));

		const int frameWidth = edit->style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
		edit->setStyleSheet (QString ("QLineEdit { padding-right: %1px; }")
					.arg (Button_->sizeHint ().width () + frameWidth + 1));
		const auto msz = edit->minimumSizeHint ();
		edit->setMinimumSize (qMax (msz.width (), Button_->sizeHint ().height () + frameWidth * 2 + 2),
						qMax (msz.height(), Button_->sizeHint ().height () + frameWidth * 2 + 2));

		UpdatePos ();

		edit->installEventFilter (this);
	}

	bool ClearLineEditAddon::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () == QEvent::Resize ||
			event->type () == QEvent::Move)
			UpdatePos ();

		return QObject::eventFilter (obj, event);
	}

	void ClearLineEditAddon::UpdatePos ()
	{
		const auto& hint = Button_->sizeHint ();
		const auto& rect = Edit_->rect ();
		const int frameWidth = Edit_->style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
		Button_->move (rect.right () - frameWidth - hint.width (),
				(rect.bottom () + 1 - hint.height ()) / 2);
	}

	void ClearLineEditAddon::updateButton (const QString& text)
	{
		Button_->setVisible (!text.isEmpty ());
	}
}
}

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

#include "unhoverdeletemixin.h"
#include <QWidget>
#include <QEvent>
#include <QTimer>

namespace LeechCraft
{
namespace Util
{
	UnhoverDeleteMixin::UnhoverDeleteMixin (QWidget *watched)
	: QObject (watched)
	, LeaveTimer_ (new QTimer (this))
	, ContainsMouse_ (false)
	{
		watched->installEventFilter (this);

		LeaveTimer_->setSingleShot (true);
		connect (LeaveTimer_,
				SIGNAL (timeout ()),
				watched,
				SLOT (deleteLater ()));
	}

	void UnhoverDeleteMixin::Start (int timeout)
	{
		if (!ContainsMouse_)
			LeaveTimer_->start (timeout);
	}

	void UnhoverDeleteMixin::Stop()
	{
		LeaveTimer_->stop ();
	}

	bool UnhoverDeleteMixin::eventFilter (QObject*, QEvent *event)
	{
		switch (event->type ())
		{
		case QEvent::Enter:
			ContainsMouse_ = true;
			LeaveTimer_->stop ();
			break;
		case QEvent::Leave:
			ContainsMouse_ = false;
			LeaveTimer_->start (800);
			break;
		default:
			break;
		}

		return false;
	}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "unhoverdeletemixin.h"
#include <QWidget>
#include <QEvent>
#include <QTimer>
#include <QtDebug>
#include <QApplication>
#include <QStyle>

namespace LC
{
namespace Util
{
	UnhoverDeleteMixin::UnhoverDeleteMixin (QObject *watched, const char *slot)
	: QObject (watched)
	, LeaveTimer_ (new QTimer (this))
	, ContainsMouse_ (false)
	{
		watched->installEventFilter (this);

		LeaveTimer_->setSingleShot (true);
		connect (LeaveTimer_,
				SIGNAL (timeout ()),
				watched,
				slot);
	}

	namespace
	{
		int DefaultTimeout ()
		{
			return QApplication::style ()->styleHint (QStyle::SH_ToolTip_WakeUpDelay) * 2;
		}
	}

	void UnhoverDeleteMixin::Start (std::optional<int> timeout)
	{
		if (!ContainsMouse_)
			LeaveTimer_->start (timeout.value_or (DefaultTimeout ()));
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
			LeaveTimer_->start (DefaultTimeout ());
			break;
		default:
			break;
		}

		return false;
	}
}
}

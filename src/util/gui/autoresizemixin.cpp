/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "autoresizemixin.h"
#include <QWidget>
#include <QWindow>
#include <QResizeEvent>
#include <util/gui/geometry.h>

namespace LC
{
namespace Util
{
	AutoResizeMixin::AutoResizeMixin (const QPoint& point, RectGetter_f size, QWidget *view)
	: QObject (view)
	, OrigPoint_ (point)
	, Mover_ ([view] (const QPoint& pos) { view->move (pos); })
	, Rect_ (size)
	{
		view->installEventFilter (this);
		Refit (view->size ());
	}

	AutoResizeMixin::AutoResizeMixin (const QPoint& point, RectGetter_f size, QWindow *window)
	: QObject (window)
	, OrigPoint_ (point)
	, Mover_ ([window] (const QPoint& pos) { window->setPosition (pos); })
	, Rect_ (size)
	{
		window->installEventFilter (this);
		Refit (window->size ());
	}

	bool AutoResizeMixin::eventFilter (QObject*, QEvent *event)
	{
		if (event->type () != QEvent::Resize)
			return false;

		auto re = static_cast<QResizeEvent*> (event);
		Refit (re->size ());
		return false;
	}

	void AutoResizeMixin::Refit (const QSize& size)
	{
		Mover_ (FitRect (OrigPoint_, size, Rect_ (), Util::FitFlag::NoOverlap));
	}
}
}

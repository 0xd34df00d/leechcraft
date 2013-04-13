/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "autoresizemixin.h"
#include <QDeclarativeView>
#include <QResizeEvent>
#include <util/gui/util.h>

namespace LeechCraft
{
namespace Util
{
	AutoResizeMixin::AutoResizeMixin (const QPoint& point, RectGetter_f size, QWidget *view)
	: QObject (view)
	, OrigPoint_ (point)
	, View_ (view)
	, Rect_ (size)
	{
		View_->installEventFilter (this);

		Refit (View_->size ());
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
		const auto& pos = FitRect (OrigPoint_, size, Rect_ (), Util::FitFlag::NoOverlap);
		View_->move (pos);
	}
}
}

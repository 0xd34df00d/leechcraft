/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "launcherdroparea.h"
#include <QCursor>
#include <QMimeData>

namespace LC::SB2
{
	LauncherDropArea::LauncherDropArea (QQuickItem *parent)
	: QQuickItem (parent)
	{
		SetAcceptingDrops (true);
	}

	bool LauncherDropArea::GetAcceptingDrops () const
	{
		return flags () & ItemAcceptsDrops;
	}

	void LauncherDropArea::SetAcceptingDrops (bool accepting)
	{
		if (GetAcceptingDrops () == accepting)
			return;

		setFlag (ItemAcceptsDrops, accepting);
		emit acceptingDropsChanged (accepting);
	}

	void LauncherDropArea::dragEnterEvent (QDragEnterEvent *event)
	{
		auto data = event->mimeData ();
		if (!data->formats ().contains (QLatin1String { "x-leechcraft/tab-tabclass" }))
			return;

		event->acceptProposedAction ();
		setCursor (Qt::DragCopyCursor);
	}

	void LauncherDropArea::dragLeaveEvent (QDragLeaveEvent*)
	{
		unsetCursor ();
	}

	void LauncherDropArea::dropEvent (QDropEvent *event)
	{
		unsetCursor ();
		emit tabDropped (event->mimeData ()->data (QStringLiteral ("x-leechcraft/tab-tabclass")));
	}
}

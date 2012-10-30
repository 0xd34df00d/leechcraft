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

#include "pagesview.h"
#include <QMenu>
#include <QMouseEvent>

namespace LeechCraft
{
namespace Monocle
{
	PagesView::PagesView (QWidget *parent)
	: QGraphicsView (parent)
	, ShowReleaseMenu_ (false)
	, ShowOnNextRelease_ (false)
	{
	}

	void PagesView::SetShowReleaseMenu (bool show)
	{
		ShowReleaseMenu_ = show;
		ShowOnNextRelease_ = false;
	}

	void PagesView::mouseMoveEvent (QMouseEvent *event)
	{
		if (ShowReleaseMenu_)
			ShowOnNextRelease_ = true;

		QGraphicsView::mouseMoveEvent (event);
	}

	void PagesView::mouseReleaseEvent (QMouseEvent *event)
	{
		QGraphicsView::mouseReleaseEvent (event);

		if (ShowOnNextRelease_)
		{
			auto menu = new QMenu;
			menu->addActions (actions ());
			menu->popup (event->globalPos ());
			menu->setAttribute (Qt::WA_DeleteOnClose);
			menu->show ();

			ShowOnNextRelease_ = false;
		}
	}

	void PagesView::resizeEvent (QResizeEvent *e)
	{
		QGraphicsView::resizeEvent (e);
		emit sizeChanged ();
	}
}
}

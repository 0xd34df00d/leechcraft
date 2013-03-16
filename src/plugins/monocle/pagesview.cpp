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

#include "pagesview.h"
#include <QMenu>
#include <QMouseEvent>
#include <QTimeLine>
#include "xmlsettingsmanager.h"
#include "documenttab.h"

namespace LeechCraft
{
namespace Monocle
{
	PagesView::PagesView (QWidget *parent)
	: QGraphicsView (parent)
	, ShowReleaseMenu_ (false)
	, ShowOnNextRelease_ (false)
	, ScrollTimeline_ (new QTimeLine (400, this))
	, DocTab_ (0)
	{
		ScrollTimeline_->setFrameRange (0, 100);
		connect (ScrollTimeline_,
				SIGNAL (frameChanged (int)),
				this,
				SLOT (handleSmoothScroll (int)));
	}

	void PagesView::SetDocumentTab (DocumentTab *tab)
	{
		DocTab_ = tab;
	}

	void PagesView::SetShowReleaseMenu (bool show)
	{
		ShowReleaseMenu_ = show;
		ShowOnNextRelease_ = false;
	}

	QPointF PagesView::GetCurrentCenter () const
	{
		const auto& rectSize = viewport ()->contentsRect ().size () / 2;
		return mapToScene (QPoint (rectSize.width (), rectSize.height ()));
	}

	void PagesView::SmoothCenterOn (qreal x, qreal y)
	{
		if (!XmlSettingsManager::Instance ().property ("SmoothScrolling").toBool ())
		{
			centerOn (x, y);
			return;
		}

		const auto& current = GetCurrentCenter ();
		XPath_ = qMakePair (current.x (), x);
		YPath_ = qMakePair (current.y (), y);

		if (ScrollTimeline_->state () != QTimeLine::NotRunning)
			ScrollTimeline_->stop ();
		ScrollTimeline_->start ();
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

			auto actions = DocTab_->CreateViewCtxMenuActions ();
			for (auto act : actions)
				connect (menu,
						SIGNAL (destroyed (QObject*)),
						act,
						SLOT (deleteLater ()));

			menu->addActions (actions);
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

	void PagesView::handleSmoothScroll (int frame)
	{
		const int endFrame = ScrollTimeline_->endFrame ();
		auto interp = [frame, endFrame] (const QPair<qreal, qreal>& pair)
				{ return pair.first + (pair.second - pair.first) * frame / endFrame; };
		centerOn (interp (XPath_), interp (YPath_));
	}
}
}

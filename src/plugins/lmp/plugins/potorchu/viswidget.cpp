/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viswidget.h"
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QTimer>
#include <QGLWidget>
#include <QtDebug>

namespace LC
{
namespace LMP
{
namespace Potorchu
{
	VisWidget::VisWidget (QWidget *parent)
	: QGraphicsView { parent }
	, Timer_ { new QTimer { this } }
	{
		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (redrawRequested ()));

		setViewport (new QGLWidget { QGLFormat { QGL::SampleBuffers } });
		setViewportUpdateMode (QGraphicsView::FullViewportUpdate);
	}

	void VisWidget::SetFps (int fps)
	{
		const bool isRunning = Timer_->isActive ();
		if (isRunning)
			Timer_->stop ();
		Timer_->setInterval (1000.0 / fps);
		if (isRunning)
			Timer_->start ();
	}

	void VisWidget::hideEvent (QHideEvent *event)
	{
		Timer_->stop ();
		QWidget::hideEvent (event);
	}

	void VisWidget::showEvent (QShowEvent *event)
	{
		Timer_->start ();
		QGraphicsView::showEvent (event);
	}

	void VisWidget::resizeEvent (QResizeEvent *event)
	{
		QGraphicsView::resizeEvent (event);

		if (scene ())
			scene ()->setSceneRect ({ { 0, 0 }, event->size () });
	}

	void VisWidget::mouseReleaseEvent (QMouseEvent *event)
	{
		switch (event->button ())
		{
		case Qt::LeftButton:
			emit nextVis ();
			break;
		case Qt::RightButton:
			emit prevVis ();
			break;
		default:
			QWidget::mouseReleaseEvent (event);
			break;
		}
	}
}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webviewsmoothscroller.h"
#include <cmath>
#include <qwebview.h>
#include <qwebpage.h>
#include <qwebframe.h>
#include <QKeyEvent>
#include <QTimer>
#include <util/sll/lambdaeventfilter.h>

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	WebViewSmoothScroller::WebViewSmoothScroller (QWebView *view)
	: QObject { view }
	, View_ { view }
	, ScrollTimer_ { new QTimer { this } }
	{
		connect (ScrollTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleAutoscroll ()));

		auto ef = Util::MakeLambdaEventFilter ([this] (QKeyEvent *event)
				{
					if (event->modifiers () == Qt::SHIFT &&
						(event->key () == Qt::Key_PageUp || event->key () == Qt::Key_PageDown))
					{
						ScrollDelta_ += event->key () == Qt::Key_PageUp ? -0.1 : 0.1;
						if (!ScrollTimer_->isActive ())
							ScrollTimer_->start (30);
						return true;
					}
					else if (event->modifiers () == Qt::SHIFT &&
							 event->key () == Qt::Key_Delete)
					{
						ScrollDelta_ = 0;
						ScrollTimer_->stop ();
						return true;
					}
					return false;
				},
				this);
		view->installEventFilter (ef);
	}

	void WebViewSmoothScroller::handleAutoscroll ()
	{
		if (std::fabs (ScrollDelta_) < std::numeric_limits<decltype (ScrollDelta_)>::epsilon ())
			return;

		AccumulatedScrollShift_ += ScrollDelta_;

		if (std::abs (AccumulatedScrollShift_) >= 1)
		{
			const auto mf = View_->page ()->mainFrame ();
			auto pos = mf->scrollPosition ();
			pos += QPoint (0, AccumulatedScrollShift_);
			mf->setScrollPosition (pos);

			AccumulatedScrollShift_ -= static_cast<int> (AccumulatedScrollShift_);
		}
	}
}
}
}

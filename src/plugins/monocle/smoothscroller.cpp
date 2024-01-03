/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "smoothscroller.h"
#include <QTimeLine>
#include "pagesview.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Monocle
{
	SmoothScroller::SmoothScroller (PagesView *view, QObject *parent)
	: QObject { parent }
	, View_ { view }
	, ScrollTimeline_ { new QTimeLine { 400, this } }
	{
		ScrollTimeline_->setFrameRange (0, 100);
		connect (ScrollTimeline_,
				&QTimeLine::frameChanged,
				this,
				&SmoothScroller::HandleSmoothScroll);

		connect (ScrollTimeline_,
				&QTimeLine::finished,
				[this] { emit isCurrentlyScrollingChanged (false); });
	}

	bool SmoothScroller::IsCurrentlyScrolling () const
	{
		return ScrollTimeline_->state () == QTimeLine::Running;
	}

	void SmoothScroller::SmoothCenterOn (qreal x, qreal y)
	{
		if (!XmlSettingsManager::Instance ().property ("SmoothScrolling").toBool ())
		{
			View_->centerOn (x, y);
			return;
		}

		const auto& current = View_->GetCurrentCenter ();
		XPath_ = qMakePair (current.x (), x);
		YPath_ = qMakePair (current.y (), y);

		if (ScrollTimeline_->state () != QTimeLine::NotRunning)
			ScrollTimeline_->stop ();

		emit isCurrentlyScrollingChanged (true);
		ScrollTimeline_->start ();
	}

	void SmoothScroller::HandleSmoothScroll (int frame)
	{
		const int endFrame = ScrollTimeline_->endFrame ();
		auto interp = [frame, endFrame] (QPair<qreal, qreal> pair)
				{ return pair.first + (pair.second - pair.first) * frame / endFrame; };
		View_->centerOn (interp (XPath_), interp (YPath_));
	}
}
}

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "smoothscroller.h"
#include <QTimeLine>
#include "components/viewitems/pagesview.h"
#include "xmlsettingsmanager.h"

namespace LC::Monocle
{
	constexpr auto Duration = 400;
	constexpr auto StepsCount = 100;

	SmoothScroller::SmoothScroller (PagesView& view, QObject *parent)
	: QObject { parent }
	, View_ { view }
	, ScrollTimeline_ { *new QTimeLine { Duration, this } }
	{
		ScrollTimeline_.setFrameRange (0, StepsCount);
		connect (&ScrollTimeline_,
				&QTimeLine::frameChanged,
				this,
				&SmoothScroller::HandleSmoothScroll);

		connect (&ScrollTimeline_,
				&QTimeLine::finished,
				this,
				[this] { emit isCurrentlyScrollingChanged (false); });
	}

	bool SmoothScroller::IsCurrentlyScrolling () const
	{
		return ScrollTimeline_.state () == QTimeLine::Running;
	}

	void SmoothScroller::SmoothCenterOn (const QGraphicsItem& item)
	{
		SmoothCenterOnPoint (View_.GetViewportTrimmedCenter (item));
	}

	void SmoothScroller::SmoothCenterOnPoint (SceneAbsolutePos p)
	{
		if (!XmlSettingsManager::Instance ().property ("SmoothScrolling").toBool ())
		{
			View_.centerOn (p.ToPointF ());
			return;
		}

		const auto& current = View_.GetCurrentCenter ();
		ScrollPath_ = qMakePair (current, p);

		if (ScrollTimeline_.state () != QTimeLine::NotRunning)
			ScrollTimeline_.stop ();

		emit isCurrentlyScrollingChanged (true);
		ScrollTimeline_.start ();
	}

	void SmoothScroller::HandleSmoothScroll (int frame)
	{
		const int endFrame = ScrollTimeline_.endFrame ();
		auto interp = [frame, endFrame] (const auto& pair)
				{ return pair.first + (pair.second - pair.first) * frame / endFrame; };
		View_.CenterOn (interp (ScrollPath_));
	}
}

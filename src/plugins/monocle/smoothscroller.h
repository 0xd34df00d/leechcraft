/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointF>

class QTimeLine;

namespace LC::Monocle
{
	class PagesView;

	class SmoothScroller : public QObject
	{
		Q_OBJECT

		PagesView * const View_;
		QTimeLine * const ScrollTimeline_;

		QPair<QPointF, QPointF> ScrollPath_;
	public:
		SmoothScroller (PagesView*, QObject* = nullptr);

		bool IsCurrentlyScrolling () const;

		void SmoothCenterOn (QPointF);
	private:
		void HandleSmoothScroll (int);
	signals:
		void isCurrentlyScrollingChanged (bool);
	};
}

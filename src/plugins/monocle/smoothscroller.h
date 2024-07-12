/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "components/layout/positions.h"

class QGraphicsItem;
class QTimeLine;

namespace LC::Monocle
{
	class PagesView;

	class SmoothScroller : public QObject
	{
		Q_OBJECT

		PagesView& View_;
		QTimeLine& ScrollTimeline_;

		QPair<SceneAbsolutePos, SceneAbsolutePos> ScrollPath_;
	public:
		explicit SmoothScroller (PagesView&, QObject* = nullptr);

		bool IsCurrentlyScrolling () const;

		void SmoothCenterOn (const QGraphicsItem&);
		void SmoothCenterOnPoint (SceneAbsolutePos);
	private:
		void HandleSmoothScroll (int);
	signals:
		void isCurrentlyScrollingChanged (bool);
	};
}

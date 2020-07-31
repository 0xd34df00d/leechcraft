/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QTimeLine;

namespace LC
{
namespace Monocle
{
	class PagesView;

	class SmoothScroller : public QObject
	{
		Q_OBJECT

		PagesView * const View_;
		QTimeLine * const ScrollTimeline_;

		QPair<qreal, qreal> XPath_;
		QPair<qreal, qreal> YPath_;
	public:
		SmoothScroller (PagesView*, QObject* = nullptr);

		bool IsCurrentlyScrolling () const;

		void SmoothCenterOn (qreal, qreal);
	private:
		void HandleSmoothScroll (int);
	signals:
		void isCurrentlyScrollingChanged (bool);
	};
}
}

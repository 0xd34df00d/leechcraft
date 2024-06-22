/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsView>
#include "components/layout/positions.h"

namespace LC::Monocle
{
	class IDocument;

	class PagesView : public QGraphicsView
	{
		Q_OBJECT

		bool ShowReleaseMenu_ = false;
		bool ShowOnNextRelease_ = false;

		IDocument *Doc_ = nullptr;
	public:
		using QGraphicsView::QGraphicsView;

		void SetDocument (IDocument*);
		void SetShowReleaseMenu (bool);

		void CenterOn (SceneAbsolutePos);

		SceneAbsolutePos GetCurrentCenter () const;
		SceneAbsolutePos GetViewportTrimmedCenter (const QGraphicsItem&) const;
	protected:
		void mouseMoveEvent (QMouseEvent*) override;
		void mouseReleaseEvent (QMouseEvent*) override;
		void resizeEvent (QResizeEvent*) override;
	signals:
		void sizeChanged ();
	};
}

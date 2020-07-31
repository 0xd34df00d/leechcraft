/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsView>

namespace LC
{
namespace Monocle
{
	class DocumentTab;

	class PagesView : public QGraphicsView
	{
		Q_OBJECT

		bool ShowReleaseMenu_ = false;
		bool ShowOnNextRelease_ = false;

		DocumentTab *DocTab_ = nullptr;
	public:
		PagesView (QWidget* = nullptr);

		void SetDocumentTab (DocumentTab*);
		void SetShowReleaseMenu (bool);

		QPointF GetCurrentCenter () const;
	protected:
		void mouseMoveEvent (QMouseEvent*);
		void mouseReleaseEvent (QMouseEvent*);
		void resizeEvent (QResizeEvent*);
	signals:
		void sizeChanged ();
	};
}
}

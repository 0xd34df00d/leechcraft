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
namespace LMP
{
namespace Potorchu
{
	class VisWidget : public QGraphicsView
	{
		Q_OBJECT

		QTimer * const Timer_;
	public:
		VisWidget (QWidget* = 0);

		void SetFps (int);
	protected:
		void hideEvent (QHideEvent*) override;
		void showEvent (QShowEvent*) override;
		void resizeEvent (QResizeEvent*) override;
		void mouseReleaseEvent (QMouseEvent*) override;
	signals:
		void prevVis ();
		void nextVis ();

		void redrawRequested ();
	};
}
}
}

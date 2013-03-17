/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QGraphicsView>

class QTimeLine;

namespace LeechCraft
{
namespace Monocle
{
	class DocumentTab;

	class PagesView : public QGraphicsView
	{
		Q_OBJECT

		bool ShowReleaseMenu_;
		bool ShowOnNextRelease_;

		QTimeLine *ScrollTimeline_;
		DocumentTab *DocTab_;

		QPair<qreal, qreal> XPath_;
		QPair<qreal, qreal> YPath_;
	public:
		PagesView (QWidget* = 0);

		void SetDocumentTab (DocumentTab*);
		void SetShowReleaseMenu (bool);

		QPointF GetCurrentCenter () const;
		void SmoothCenterOn (qreal, qreal);
	protected:
		void mouseMoveEvent (QMouseEvent*);
		void mouseReleaseEvent (QMouseEvent*);
		void resizeEvent (QResizeEvent*);
	private slots:
		void handleSmoothScroll (int);
	signals:
		void sizeChanged ();
	};
}
}

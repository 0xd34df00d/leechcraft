/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTabBar>
#include <QHash>

namespace LC
{
	class MainWindow;
	class SeparateTabWidget;

	struct TabInfo;

	class SeparateTabBar : public QTabBar
	{
		Q_OBJECT

		MainWindow *Window_ = nullptr;

		bool InMove_ = false;
		SeparateTabWidget *TabWidget_ = nullptr;

		QPoint DragStartPos_;

		QWidget *AddTabButton_ = nullptr;
		mutable QVector<int> ComputedWidths_;
	public:
		explicit SeparateTabBar (QWidget* = 0);

		void SetWindow (MainWindow*);

		void SetTabClosable (int index, bool closable, QWidget *closeButton = 0);
		void SetTabWidget (SeparateTabWidget*);

		void SetAddTabButton (QWidget*);

		QTabBar::ButtonPosition GetCloseButtonPosition () const;

		void SetInMove (bool inMove);
	private:
		QTabBar::ButtonPosition GetAntiCloseButtonPosition () const;

		QVector<TabInfo> GetTabInfos () const;
		void UpdateComputedWidths () const;
	private slots:
		void toggleCloseButtons () const;
	protected:
		void tabLayoutChange ();
		QSize tabSizeHint (int) const;

		void mouseReleaseEvent (QMouseEvent*);

		void mousePressEvent (QMouseEvent*);
		void mouseMoveEvent (QMouseEvent*);
		void dragEnterEvent (QDragEnterEvent*);
		void dragMoveEvent (QDragMoveEvent*);
		void dropEvent (QDropEvent*);

		void mouseDoubleClickEvent (QMouseEvent*);

		void tabInserted (int);
		void tabRemoved (int);
	signals:
		void addDefaultTab ();
		void tabWasInserted (int);
		void tabWasRemoved (int);
		void releasedMouseAfterMove (int index);
	};
}

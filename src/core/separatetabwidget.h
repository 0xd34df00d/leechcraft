/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <QIcon>
#include <QTabBar>
#include <QPointer>
#include <QAction>
#include <QMap>
#include <QMenu>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoretabwidget.h>
#include "interfaces/core/ihookproxy.h"

class QMenu;
class QStackedWidget;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QToolBar;

namespace LC
{
	class MainWindow;
	class SeparateTabBar;

	class SeparateTabWidget : public QWidget
							, public ICoreTabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ICoreTabWidget)

		MainWindow *Window_ = 0;

		int LastContextMenuTab_ = -1;
		QMenu *AddTabButtonContextMenu_;
		QPointer<QStackedWidget> MainStackedWidget_;
		QPointer<SeparateTabBar> MainTabBar_;
		QPointer<QToolButton> AddTabButton_;
		QPointer<QToolBar> LeftToolBar_;
		QPointer<QToolBar> RightToolBar_;
		QPointer<QHBoxLayout> MainTabBarLayout_;
		QPointer<QHBoxLayout> MainToolBarLayout_;
		QVBoxLayout *MainLayout_;
		QAction *DefaultTabAction_;
		QList<QPointer<QAction>> TabBarActions_;

		QWidget *CurrentWidget_ = 0;
		int CurrentIndex_ = -1;
		QWidget *PreviousWidget_ = 0;
		QToolBar *CurrentToolBar_ = 0;

		QStringList TabNames_;

		QHash<QWidget*, QPointer<QWidget>> SavedWidgetParents_;
	public:
		explicit SeparateTabWidget (QWidget *parent = 0);
		void SetWindow (MainWindow*);

		QObject* GetQObject ();

		int WidgetCount () const;
		QWidget* Widget (int index) const;

		QList<QAction*> GetPermanentActions () const;

		QString TabText (int index) const;
		void SetTabText (int index, const QString& text);

		QIcon TabIcon (int index) const;
		void SetTabIcon (int index, const QIcon& icon);

		QString TabToolTip (int index) const;
		void SetTabToolTip (int index, const QString& toolTip);

		QWidget* TabButton (int index, QTabBar::ButtonPosition positioin) const;
		QTabBar::ButtonPosition GetCloseButtonPosition () const;
		void SetTabClosable (int index, bool closable, QWidget *closeButton = 0);

		void SetTabsClosable (bool closable);

		void AddAction2TabBar (QAction *action);
		void InsertAction2TabBar (int position, QAction *action);
		void InsertAction2TabBar (QAction *before, QAction *action);

		void AddWidget2TabBarLayout (QTabBar::ButtonPosition pos, QWidget *action);
		void AddAction2TabBarLayout (QTabBar::ButtonPosition pos, QAction *action);
		void InsertAction2TabBarLayout (QTabBar::ButtonPosition pos, QAction *action, int index);
		void RemoveActionFromTabBarLayout (QTabBar::ButtonPosition pos, QAction *action);

		int CurrentIndex () const;
		QWidget* CurrentWidget () const;

		QMenu* GetTabMenu (int);

		int IndexOf (QWidget *page) const;

		int GetLastContextMenuTab () const;

		void SetAddTabButtonContextMenu (QMenu *menu);

		SeparateTabBar* TabBar () const;

		int AddTab (QWidget *page, const QString& text);
		int AddTab (QWidget *page, const QIcon& icon, const QString& text);
		int InsertTab (int index, QWidget *page, const QString& text);
		int InsertTab (int index, QWidget *page,
				const QIcon& icon, const QString& text);
		void RemoveTab (int index);

		void AddWidget2SeparateTabWidget (QWidget *widget);
		void RemoveWidgetFromSeparateTabWidget (QWidget *widget);

		int TabAt (const QPoint& point);

		void MoveTab (int from, int to);

		QWidget* GetPreviousWidget () const;
	protected:
		void resizeEvent (QResizeEvent *event);
		void mousePressEvent (QMouseEvent *event);
	private:
		void Init ();
		void AddTabButtonInit ();
	public slots:
		void setCurrentIndex (int index);
		void setCurrentTab (int tabIndex);
		void setCurrentWidget (QWidget *widget);
		void handleNewTabShortcutActivated ();
		void setPreviousTab ();
	private slots:
		void handleTabBarPosition ();
		void handleSelectionBehavior ();
		void handleAddDefaultTab ();
		void handleCloneTab ();
		void handleTabMoved (int from, int to);
		void handleContextMenuRequested (const QPoint& point);
		void handleActionDestroyed ();
		void releaseMouseAfterMove (int index);
	signals:
		void tabInserted (int index);
		void tabWasRemoved (int index);
		void tabCloseRequested (int index);
		void newTabMenuRequested ();
		void currentChanged (int index);
		void tabWasMoved (int from, int to);

		// Hook
		void hookTabContextMenuFill (LC::IHookProxy_ptr proxy,
				QMenu *menu, int index, int windowsId);
		void hookTabFinishedMoving (LC::IHookProxy_ptr proxy,
				int index,
				int windowId);
		void hookTabSetText (LC::IHookProxy_ptr proxy,
				int index,
				int windowId);
		void hookTabIsRemoving (LC::IHookProxy_ptr proxy,
				int index,
				int windowId);
	};
}

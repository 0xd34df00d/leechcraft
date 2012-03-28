/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef SEPARATETABWIDGET_H
#define SEPARATETABWIDGET_H
#include <memory>
#include <QWidget>
#include <QIcon>
#include <QTabBar>
#include <QPointer>
#include <QAction>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoretabwidget.h>
#include "interfaces/core/ihookproxy.h"

class QMenu;
class QStackedWidget;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QToolBar;

namespace LeechCraft
{
	class SeparateTabBar;

	class SeparateTabWidget : public QWidget
							, public ICoreTabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ICoreTabWidget)

		int LastContextMenuTab_;
		QMenu *DefaultContextMenu_;
		QMenu *AddTabButtonContextMenu_;
		QPointer<QStackedWidget> MainStackedWidget_;
		QPointer<SeparateTabBar> MainTabBar_;
		QPointer<QToolButton> AddTabButton_;
		QPointer<QToolBar> LeftToolBar_;
		QPointer<QToolBar> RightToolBar_;
		QPointer<QHBoxLayout> MainTabBarLayout_;
		QPointer<QHBoxLayout> MainToolBarLayout_;
		QVBoxLayout *MainLayout_;
		QWidget *DefaultWidget_;
		QAction *AddTabButtonAction_;
		QAction *DefaultTabAction_;
		QMap<int, std::shared_ptr<QWidget>> Widgets_;
		QList<QPointer<QAction>> TabBarActions_;
		QWidget *CurrentWidget_;
		int CurrentIndex_;
		QWidget *PreviousWidget_;
	public:
		explicit SeparateTabWidget (QWidget *parent = 0);

		QObject* GetObject ();

		int WidgetCount () const;
		QWidget* Widget (int index) const;

		QList<QAction*> GetPermanentActions () const;

		QVariant TabData (int index) const;
		void SetTabData (int index, QVariant data);

		QString TabText (int index) const;
		void SetTabText (int index, const QString& text);

		QIcon TabIcon (int index) const;
		void SetTabIcon (int index, const QIcon& icon);

		QString TabToolTip (int index) const;
		void SetTabToolTip (int index, const QString& toolTip);

		void SetTooltip (int index, QWidget *toolTip);

		QWidget* TabButton (int index, QTabBar::ButtonPosition positioin) const;
		QTabBar::ButtonPosition GetCloseButtonPosition () const;
		void SetTabClosable (int index, bool closable, QWidget *closeButton = 0);

		void SetTabsClosable (bool closable);

		void AddAction2TabBar (QAction *action);
		void InsertAction2TabBar (int position, QAction *action);
		void InsertAction2TabBar (QAction *before, QAction *action);

		void AddWidget2TabBarLayout (QTabBar::ButtonPosition pos, QWidget *action);
		void AddAction2TabBarLayout (QTabBar::ButtonPosition pos, QAction *action);

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

		bool IsAddTabActionVisible () const;

		void AddWidget2SeparateTabWidget (QWidget *widget);
		void RemoveWidgetFromSeparateTabWidget (QWidget *widget);

		int TabAt (const QPoint& point);

		void MoveTab (int from, int to);

		QWidget* GetPreviousWidget () const;
	protected:
		void resizeEvent (QResizeEvent *event);
		bool event (QEvent *event);
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
		void handleSelectionBehavior ();
		void handleAddDefaultTab ();
		void handleShowAddTabButton (bool show);
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
		void hookTabContextMenuFill (LeechCraft::IHookProxy_ptr proxy,
				QMenu *menu, int index);
		void hookTabFinishedMoving (LeechCraft::IHookProxy_ptr proxy, int index);
		void hookTabSetText (LeechCraft::IHookProxy_ptr proxy, int index);
	};
}
#endif // SEPARATETABWIDGET_H

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

#include <QWidget>
#include <QIcon>
#include <QTabBar>
#include <QPointer>
#include <QAction>
#include <QMap>
#include <QStack>
#include <interfaces/iinfo.h>

class QStackedWidget;
class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class QToolBar;
class QMenu;

namespace LeechCraft
{

	class SeparateTabBar;

	class SeparateTabWidget : public QWidget
							, public ICoreTabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ICoreTabWidget)

		int LastContextMenuTab_;
		int PreviousTab_;
		int CurrentTab_;
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
		QAction *PinTab_;
		QAction *UnPinTab_;
		QAction *DefaultTabAction_;
		QMap<int, QWidget*> Widgets_;
		QList<QPointer<QAction> > TabBarActions_;
		bool InMoveProcess_;
	public:
		explicit SeparateTabWidget (QWidget* = 0);
		QObject* GetObject ();

		int WidgetCount () const;
		void Clear ();

		int CurrentIndex () const;
		QWidget* CurrentWidget () const;
		int IndexOf (QWidget*) const;

		int AddTab (QWidget*, const QString&);
		int AddTab (QWidget*, const QIcon&, const QString&);
		int InsertTab (int, QWidget*, const QString&);
		int InsertTab (int, QWidget*, const QIcon&, const QString&);
		void RemoveTab (int);

		void SetTabEnabled (int, bool);
		void SetTabIcon (int, const QIcon&);
		void SetTabText (int, const QString&);
		void SetTabToolTip (int, const QString&);
		void SetTabWhatsThis (int, const QString&);
		void SetTabsClosable (bool closable);
		void SetTooltip (int, QWidget*);

		bool IsTabEnabled (int) const;
		QIcon TabIcon (int) const;
		QString TabText (int) const;
		QString TabToolTip (int) const;
		QString TabWhatsThis (int) const;
		QWidget* Widget (int) const;
		int TabAt (const QPoint&);

		void SetDefaultContextMenu (QMenu*);
		QMenu* GetDefaultContextMenu () const;
		void SetAddTabButtonContextMenu (QMenu*);
		QMenu* GetAddTabButtonContextMenu () const;

		void AddWidget2TabBarLayout (QTabBar::ButtonPosition, QWidget*);
		void AddAction2TabBarLayout (QTabBar::ButtonPosition, QAction*);
		void AddWidget2SeparateTabWidget (QWidget*);
		void RemoveWidgetFromSeparateTabWidget (QWidget*);
		void SetToolBarVisible (bool);

		SeparateTabBar* TabBar () const;

		void AddAction2TabBar (QAction*);
		void InsertAction2TabBar (int, QAction*);
		void InsertAction2TabBar (QAction *before, QAction *action);

		int GetLastContextMenuTab () const;
		bool IsAddTabActionVisible () const;
		bool IsPinTab (int) const;

		bool IsInMoveProcess () const;
		void SetInMoveProcess (bool);
	protected:
		void resizeEvent (QResizeEvent*);
		bool event (QEvent*);
	private:
		void Init ();
		void AddTabButtonInit ();
		void PinTabActionsInit ();
	public slots:
		void setCurrentIndex (int);
		void setCurrentWidget (QWidget*);
		void handleNewTabShortcutActivated ();
		void on_PinTab__triggered (bool);
		void on_UnPinTab__triggered (bool);
		void setPreviousTab ();
	private slots:
		void handleCurrentChanged (int);
		void handleTabMoved (int, int);
		void handleContextMenuRequested (const QPoint&);
		void handleShowAddTabButton (bool);
		void handleAddDefaultTab (bool);
		void handleActionDestroyed ();
		void handleSelectionBehavior ();
	signals:
		void newTabRequested ();
		void newTabMenuRequested ();
		void tabWasMoved (int, int);
		void currentChanged (int);
		void tabCloseRequested (int);
		void tabWasInserted (int);
		void tabWasRemoved (int);
	};
}
#endif // SEPARATETABWIDGET_H

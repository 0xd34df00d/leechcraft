/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef TABMANAGER_H
#define TABMANAGER_H
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QIcon>

class QAction;
class QIcon;
class QKeyEvent;
class QToolBar;
class QMenu;

namespace LC
{
	class SeparateTabWidget;
	class MainWindow;

	class TabManager : public QObject
	{
		Q_OBJECT

		SeparateTabWidget *TabWidget_;
		MainWindow *Window_;
		QStringList OriginalTabNames_;
		QList<QKeyEvent*> Events_;
		QMap<QWidget*, QObject*> EmbedTabs_;
		QMenu *NewTabMenu_;
		QMap<QString, QList<QAction*>> Menus_;
	public:
		TabManager (SeparateTabWidget*, MainWindow*, QObject* = 0);

		QWidget* GetCurrentWidget () const;
		QWidget* GetWidget (int) const;
		QToolBar* GetToolBar (int) const;
		int GetWidgetCount () const;
		void ForwardKeyboard (QKeyEvent*);

		int FindTabForWidget (QWidget*) const;
	public slots:
		void rotateLeft ();
		void rotateRight ();
		void navigateToTabNumber ();

		void add (const QString&, QWidget*);
		void add (const QString&, QWidget*,
				QIcon icon);
		void remove (QWidget*);

		/** Removes the tab at the given index. Calls
		 * remove(QWidget*) internally.
		 */
		void remove (int index);

		/** Translates the given widget into index and calls
		 * remove(int) internally.
		 */
		void removeByContents (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void bringToFront (QWidget*) const;
		void handleCurrentChanged (int);
		void handleMoveHappened (int, int);
		void handleCloseAllButCurrent ();
	private:
		void InvalidateName ();

		QStringList GetOriginalNames () const;
		void SetOriginalNames (const QStringList&);
	signals:
		void currentTabChanged (QWidget*);
	};
};

#endif


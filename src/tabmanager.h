/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef TABMANAGER_H
#define TABMANAGER_H
#include <QObject>
#include <QStringList>
#include <QMap>

class QAction;
class QIcon;
class QKeyEvent;
class QToolBar;
class QMenu;

namespace LeechCraft
{
	class SeparateTabWidget;

	class TabManager : public QObject
	{
		Q_OBJECT

		SeparateTabWidget *TabWidget_;
		QStringList OriginalTabNames_;
		QList<QKeyEvent*> Events_;
		QMap<QWidget*, QObject*> EmbedTabs_;
		QMenu *NewTabMenu_;
		QMap<QString, QList<QAction*> > Menus_;
	public:
		TabManager (SeparateTabWidget*, QObject* = 0);

		QWidget* GetWidget (int) const;
		QToolBar* GetToolBar (int) const;
		void ForwardKeyboard (QKeyEvent*);
	public slots:
		void rotateLeft ();
		void rotateRight ();
		void navigateToTabNumber ();

		void add (const QString&, QWidget*);
		void add (const QString&, QWidget*,
				const QIcon& icon);
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
		void changeTooltip (QWidget*, QWidget*);
		void bringToFront (QWidget*) const;
		void handleScrollButtons ();
		void handleCurrentChanged (int);
		void handleMoveHappened (int, int);
		void handleCloseAllButCurrent ();
	private:
		int FindTabForWidget (QWidget*) const;
		QString MakeTabName (const QString&) const;
		void InvalidateName ();
	};
};

#endif


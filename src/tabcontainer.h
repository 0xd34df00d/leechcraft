/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef TABCONTAINER_H
#define TABCONTAINER_H
#include <QObject>
#include <QStringList>
#include <QMap>

class QIcon;
class QKeyEvent;
class QToolBar;
class QMenu;

namespace LeechCraft
{
	class TabWidget;

	class TabContainer : public QObject
	{
		Q_OBJECT

		TabWidget *TabWidget_;
		QStringList OriginalTabNames_;
		QList<QKeyEvent*> Events_;
		QMap<QWidget*, QToolBar*> StaticBars_;
		QMap<QWidget*, QObject*> EmbedTabs_;
		QList<QObject*> RegisteredMultiTabs_;
		QMenu *NewTabMenu_;
		QMenu *RestoreMenu_;
	public:
		TabContainer (TabWidget*, QObject* = 0);
		virtual ~TabContainer ();

		QWidget* GetWidget (int) const;
		QToolBar* GetToolBar (int) const;
		QMenu* GetNewTabMenu () const;
		void SetToolBar (QToolBar*, QWidget*);
		void RotateLeft ();
		void RotateRight ();
		void ForwardKeyboard (QKeyEvent*);

		void AddObject (QObject*);
	public slots:
		void add (const QString&, QWidget*);
		void add (const QString&, QWidget*,
				const QIcon& icon);
		void remove (QWidget*);
		void remove (int);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void bringToFront (QWidget*) const;
		void handleScrollButtons ();
		void handleCurrentChanged (int);
		void handleMoveHappened (int, int);
		void handleCloseAllButCurrent ();
	private slots:
		void restoreEmbedTab ();
	private:
		int FindTabForWidget (QWidget*) const;
		QString MakeTabName (const QString&) const;
		void InvalidateName ();
	};
};

#endif


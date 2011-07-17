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

#ifndef NEWTABMENUMANAGER_H
#define NEWTABMENUMANAGER_H
#include <QObject>
#include <QMap>
#include <QString>
#include <QSet>

class QMenu;
class QAction;

class ITabWidget;

namespace LeechCraft
{
	class NewTabMenuManager : public QObject
	{
		Q_OBJECT

		QMenu *NewTabMenu_;
		QMenu *AdditionalTabMenu_;
		QList<QObject*> RegisteredMultiTabs_;
		QSet<QChar> UsedAccelerators_;
		QMap<QObject*, QMap<QString, QAction*> > HiddenActions_;
	public:
		NewTabMenuManager (QObject* = 0);

		void AddObject (QObject*);
		void HandleEmbedTabRemoved (QObject*);
		void SetToolbarActions (QList<QList<QAction*> >);
		void SingleRemoved (ITabWidget*);

		QMenu* GetNewTabMenu () const;
		QMenu* GetAdditionalMenu ();
	private:
		QString AccelerateName (QString);
		void ToggleHide (QObject*, const QByteArray&, bool);
		void OpenTab (QAction*);
		void InsertAction (QAction*);
	private slots:
		void handleNewTabRequested ();
	signals:
		void restoreTabActionAdded (QAction*);
	};
}

#endif

/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

class QMenu;
class QAction;

namespace LeechCraft
{
	class NewTabMenuManager : public QObject
	{
		Q_OBJECT

		QMenu *NewTabMenu_;
		QList<QObject*> RegisteredMultiTabs_;
		QMap<QString, QAction*> ReaddOnRestore_;
	public:
		NewTabMenuManager (QObject* = 0);

		void AddObject (QObject*);
		void HandleEmbedTabRemoved (QObject*);

		QMenu* GetNewTabMenu () const;
	private slots:
		void restoreEmbedTab ();
	signals:
		void restoreEmbedTabRequested (QObject*);
	};
}

#endif

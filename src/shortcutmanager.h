/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H
#include <QDialog>
#include "interfaces/ihaveshortcuts.h"
#include "ui_shortcutmanager.h"

class QSortFilterProxyModel;
class QStandardItemModel;

namespace LeechCraft
{
	class ShortcutManager : public QWidget
						  , public IShortcutProxy
	{
		Q_OBJECT
		Q_INTERFACES (IShortcutProxy);

		Ui::ShortcutManager Ui_;
		QStandardItemModel *Model_;
		QSortFilterProxyModel *Filter_;
	public:
		enum Roles
		{
			Object = Qt::UserRole + 1,
			OriginalName,
			Sequence,
			OldSequence
		};

		ShortcutManager (QWidget* = 0);
		void AddObject (QObject*);
		void AddObject (QObject*, const QString&,
				const QString&, const QIcon&);
		QKeySequences_t GetShortcuts (QObject*, const QString&);
	public slots:
		void on_Tree__activated (const QModelIndex&);
		virtual void accept ();
		virtual void reject ();
	};
};

#endif


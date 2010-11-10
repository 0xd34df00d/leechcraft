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

#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H
#include <QDialog>
#include "ui_shortcutmanager.h"
#include "interfaces/ihaveshortcuts.h"

namespace LeechCraft
{
	class ShortcutManager : public QWidget
						  , public IShortcutProxy
	{
		Q_OBJECT
		Q_INTERFACES (IShortcutProxy);

		Ui::ShortcutManager Ui_;
		enum
		{
			RoleObject = 50,
			RoleOriginalName,
			RoleSequence,
			RoleOldSequence
		};
	public:
		ShortcutManager (QWidget* = 0);
		void AddObject (QObject*);
		void AddObject (QObject*, const QString&,
				const QString&, const QIcon&);
		QKeySequences_t GetShortcuts (const QObject*, const QString&) const ;
	public slots:
		void on_Tree__itemActivated (QTreeWidgetItem*);
		virtual void accept ();
		virtual void reject ();
	};
};

#endif


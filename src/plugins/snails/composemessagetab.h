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

#ifndef PLUGINS_SNAILS_COMPOSEMESSAGETAB_H
#define PLUGINS_SNAILS_COMPOSEMESSAGETAB_H
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_composemessagetab.h"

namespace LeechCraft
{
namespace Snails
{
	class ComposeMessageTab : public QWidget
							, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		static QObject *S_ParentPlugin_;
		static TabClassInfo S_TabClassInfo_;

		Ui::ComposeMessageTab Ui_;

		QToolBar *Toolbar_;
		QMenu *AccountsMenu_;
	public:
		static void SetParentPlugin (QObject*);
		static void SetTabClassInfo (const TabClassInfo&);

		ComposeMessageTab (QWidget* = 0);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs();
		void Remove();
		QToolBar* GetToolBar() const;
	private slots:
		void handleSend ();
	signals:
		void removeTab (QWidget*);
	};
}
}

#endif

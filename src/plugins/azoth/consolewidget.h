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

#ifndef PLUGINS_AZOTH_CONSOLEWIDGET_H
#define PLUGINS_AZOTH_CONSOLEWIDGET_H
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "interfaces/azoth/ihaveconsole.h"
#include "ui_consolewidget.h"

namespace LeechCraft
{
namespace Azoth
{
	class IAccount;
	class IHaveConsole;

	class ConsoleWidget : public QWidget
						, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget);

		Ui::ConsoleWidget Ui_;
		QObject *ParentMultiTabs_;
		TabClassInfo TabClass_;

		IAccount *AsAccount_;
		IHaveConsole *AsConsole_;
		const IHaveConsole::PacketFormat Format_;
	public:
		ConsoleWidget (QObject*, QWidget* = 0);
		
		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
		
		void SetParentMultiTabs (QObject*);
		QString GetTitle () const;
	private slots:
		void handleConsolePacket (QByteArray, int);
		void on_ClearButton__released ();
		void on_EnabledBox__toggled (bool);
	signals:
		void removeTab (QWidget*);
	};
}
}

#endif

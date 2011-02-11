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

#ifndef PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CHATHISTORYWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CHATHISTORYWIDGET_H
#include <QWidget>
#include <interfaces/imultitabs.h>
#include "ui_chathistorywidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	class Plugin;

	class ChatHistoryWidget : public QWidget
							, public IMultiTabsWidget
	{
		Q_OBJECT
		Q_INTERFACES (IMultiTabsWidget);

		Ui::ChatHistoryWidget Ui_;
		static Plugin *S_ParentMultiTabs_;
	public:
		static void SetParentMultiTabs (Plugin*);

		ChatHistoryWidget (QWidget* = 0);
		
		void Remove ();
		QToolBar* GetToolBar () const;
		void NewTabRequested ();
		QObject* ParentMultiTabs () const;
		QList<QAction*> GetTabBarContextMenuActions () const;
	private slots:
		void handleGotOurAccounts (const QStringList&);
		void handleGotUsersForAccount (const QStringList&, const QString&);
		void on_AccountBox__currentIndexChanged (int);
	signals:
		void removeSelf (QWidget*);
	};
}
}
}

#endif

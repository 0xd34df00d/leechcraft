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

#ifndef PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CHATHISTORYWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_CHATHISTORY_CHATHISTORYWIDGET_H
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_chathistorywidget.h"

class QStandardItemModel;
class QSortFilterProxyModel;

namespace LeechCraft
{
namespace Azoth
{
class ICLEntry;
namespace ChatHistory
{
	class Plugin;

	class ChatHistoryWidget : public QWidget
							, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget);

		Ui::ChatHistoryWidget Ui_;
		QStandardItemModel *ContactsModel_;
		QSortFilterProxyModel *SortFilter_;
		int Backpages_;
		int Amount_;
		int SearchShift_;
		int SearchResultPosition_;
		bool ContactSelectedAsGlobSearch_;
		QString CurrentAccount_;
		QString CurrentEntry_;
		QString PreviousSearchText_;
		QToolBar *Toolbar_;
		
		QHash<QString, QString> EntryID2NameCache_;
		
		ICLEntry *EntryToFocus_;

		static Plugin *S_ParentMultiTabs_;

		enum ModelRoles
		{
			MRIDRole = Qt::UserRole + 1
		};
	public:
		static void SetParentMultiTabs (Plugin*);

		ChatHistoryWidget (ICLEntry* = 0, QWidget* = 0);
		
		void Remove ();
		QToolBar* GetToolBar () const;
		QObject* ParentMultiTabs ();
		TabClassInfo GetTabClassInfo () const;
		QList<QAction*> GetTabBarContextMenuActions () const;
	private slots:
		void handleGotOurAccounts (const QStringList&);
		void handleGotUsersForAccount (const QStringList&, const QString&, const QStringList&);
		void handleGotChatLogs (const QString&, const QString&, int, int, const QVariant&);
		void handleGotSearchPosition (const QString&, const QString&, int);
		void on_AccountBox__currentIndexChanged (int);
		void handleContactSelected (const QModelIndex&);
		void on_HistorySearch__returnPressed ();
		void on_SearchType__currentIndexChanged ();
		void previousHistory ();
		void nextHistory ();
		void clearHistory ();
	private:
		void RequestLogs ();
		void RequestSearch ();
	signals:
		void removeSelf (QWidget*);
	};
}
}
}

#endif

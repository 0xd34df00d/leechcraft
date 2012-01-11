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

#ifndef PLUGINS_NETSTOREMANAGER_MANAGERTAB_H
#define PLUGINS_NETSTOREMANAGER_MANAGERTAB_H
#include <functional>
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_managertab.h"

class QStandardItemModel;
class QStandardItem;
class QAction;

namespace LeechCraft
{
namespace NetStoreManager
{
	class IStorageAccount;
	class ISupportFileListings;
	class AccountsManager;

	class ManagerTab : public QWidget
					 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::ManagerTab Ui_;

		QObject *Parent_;
		TabClassInfo Info_;

		AccountsManager *AM_;
		QStandardItemModel *Model_;

		QAction *ProlongateFile_;
		QAction *DeleteFile_;
	public:
		ManagerTab (const TabClassInfo&, AccountsManager*, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		IStorageAccount* GetCurrentAccount () const;
		void CallOnSelection (std::function<void (ISupportFileListings*, const QList<QStringList>&)>);
		void ClearFilesModel ();
	private slots:
		void handleGotListing (const QList<QList<QStandardItem*>>&);
		void flCopyURL ();
		void flProlongate ();
		void flDelete ();
		void on_AccountsBox__activated (int);
		void on_Update__released ();
		void on_Upload__released ();
	signals:
		void removeTab (QWidget*);

		void uploadRequested (IStorageAccount*, const QString&);
	};
}
}

#endif

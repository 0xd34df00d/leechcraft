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
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
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

	enum Columns
	{
		FirstColumnNumber
	};

	class ManagerTab : public QWidget
					 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::ManagerTab Ui_;

		QObject *Parent_;
		TabClassInfo Info_;
		ICoreProxy_ptr Proxy_;

		AccountsManager *AM_;
		QStandardItemModel *Model_;

		QAction *CopyURL_;
		QAction *DeleteFile_;
		QAction *MoveToTrash_;
		QAction *UntrashFile_;
		QAction *EmptyTrash_;
		QAction *CreateDir_;
		QHash<IStorageAccount*, QHash<QString, bool>> Account2ItemExpandState_;
	public:
		ManagerTab (const TabClassInfo&, AccountsManager*, ICoreProxy_ptr, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		IStorageAccount* GetCurrentAccount () const;
		void CallOnSelection (std::function<void (ISupportFileListings*, const QList<QStringList>&)>);
		void ClearFilesModel ();
		void SaveModelState (const QModelIndex& parent = QModelIndex ());
		void RestoreModelState ();
		void ExpandModelItems (const QModelIndex& parent = QModelIndex ());
		QList<QStringList> GetTrashedFiles () const;
	private slots:
		void handleGotListing (const QList<QList<QStandardItem*>>&);
		void handleGotFileUrl (const QUrl& url, const QList<QStringList>& id);
		void flCopyURL ();
		void flDelete ();
		void flMoveToTrash ();
		void flRestoreFromTrash ();
		void flEmptyTrash ();
		void flCreateDir ();
		void on_AccountsBox__activated (int);
		void on_Update__released ();
		void on_Upload__released ();
		void handleContextMenuRequested (const QPoint& point);
	signals:
		void removeTab (QWidget*);

		void uploadRequested (IStorageAccount*, const QString&);

		void gotEntity (LeechCraft::Entity entity);
	};
}
}

#endif

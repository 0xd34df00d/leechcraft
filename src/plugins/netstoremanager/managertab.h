/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

class QComboBox;
class QStandardItem;
class QAction;

namespace LeechCraft
{
namespace NetStoreManager
{
	class IStorageAccount;
	class ISupportFileListings;
	struct StorageItem;
	class AccountsManager;
	class FilesProxyModel;
	class FilesTreeModel;
	class FilesListModel;

	enum Columns
	{
		Name,
		Modify
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

		QToolBar *ToolBar_;

		AccountsManager *AM_;
		FilesProxyModel *ProxyModel_;
		FilesTreeModel *TreeModel_;
		FilesListModel *ListModel_;

		enum ViewMode
		{
			VMTree = 0,
			VMList
		};
		ViewMode ViewMode_;
		QAction *ViewModeAction_;

		QComboBox *AccountsBox_;

		QAction *Refresh_;
		QAction *Upload_;

		QHash<QByteArray, StorageItem*> Id2Item_;

		QAction *CopyURL_;
		QAction *DeleteFile_;
		QAction *MoveToTrash_;
		QAction *UntrashFile_;
		QAction *EmptyTrash_;
		QAction *CreateDir_;
		QAction *UploadInCurrentDir_;
		QAction *Download_;
		QHash<IStorageAccount*, QHash<QByteArray, bool>> Account2ItemExpandState_;
	public:
		ManagerTab (const TabClassInfo&, AccountsManager*, ICoreProxy_ptr, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void FillToolbar ();
		void ShowAccountActions (bool show);
		IStorageAccount* GetCurrentAccount () const;

		void ClearModel ();
		void FillModel (IStorageAccount *acc);
		void FillTreeModel (IStorageAccount *acc);
		void FillListModel (IStorageAccount *acc);

		void requestFileListings (IStorageAccount *acc);
		void requestFileChanges (IStorageAccount *acc);

		QList<QByteArray> GetTrashedFiles () const;

		void CallOnSelection (std::function<void (ISupportFileListings *sfl, const QList<QByteArray>& ids)>);

		void SaveExpandState (const QModelIndex& parent = QModelIndex ());
		void RestoreExpandState ();
		void ExpandModelItems (const QModelIndex& parent = QModelIndex ());

	private slots:
		void changeViewMode (bool set);
		void handleRefresh ();
		void handleUpload ();

		void handleAccountAdded (QObject *accObj);
		void handleAccountRemoved (QObject *accObj);

		void handleGotListing (const QList<StorageItem*>& items);

		void handleFilesViewSectionResized (int index, int oldSize, int newSize);

		void handleItemsAboutToBeCopied (const QList<QByteArray>& ids,
				const QByteArray& parentId);
		void handleItemsAboutToBeMoved (const QList<QByteArray>& ids,
				const QByteArray& parentId);
		void handleItemsAboutToBeRestoredFromTrash (const QList<QByteArray>& ids);
		void handleItemsAboutToBeTrashed (const QList<QByteArray>& ids);

		void flDelete ();
		void flMoveToTrash ();
		void flRestoreFromTrash ();
		void flEmptyTrash ();
		void flCreateDir ();
		void flUploadInCurrentDir ();
		void flDownload ();
		void flCopyUrl ();

		void handleContextMenuRequested (const QPoint& point);
		void handleExportMenuTriggered (QAction *action);

		void handleCurrentIndexChanged (int index);

		void handleGotFileUrl (const QUrl& url, const QByteArray& id = QByteArray ());

	signals:
		void removeTab (QWidget*);

		void uploadRequested (IStorageAccount *acc, const QString& fileName,
				const QByteArray& parentId = QByteArray ());
	};
}
}

#endif

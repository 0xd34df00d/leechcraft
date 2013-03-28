/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
	class AccountsManager;

	class FilesTreeModel;
	class FilesListModel;

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

		QToolBar *ToolBar_;

		AccountsManager *AM_;
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



		QAction *CopyURL_;
		QAction *DeleteFile_;
		QAction *MoveToTrash_;
		QAction *UntrashFile_;
		QAction *EmptyTrash_;
		QAction *CreateDir_;
		QAction *UploadInCurrentDir_;
		QAction *Download_;
		QHash<IStorageAccount*, QHash<QString, bool>> Account2ItemExpandState_;
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
		void CallOnSelection (std::function<void (ISupportFileListings*, const QList<QStringList>&)>);
		void ClearFilesModel ();
		void SaveModelState (const QModelIndex& parent = QModelIndex ());
		void RestoreModelState ();
		void ExpandModelItems (const QModelIndex& parent = QModelIndex ());
		QList<QStringList> GetTrashedFiles () const;
		QStandardItem* GetItemFromId (const QStringList& id) const;

	private slots:
		void changeViewMode (bool set);
		void handleRefresh ();
		void handleUpload ();

		void handleAccountAdded (QObject *accObj);
		void handleAccountRemoved (QObject *accObj);


		void handleGotListing (const QList<QList<QStandardItem*>>&);
		void handleGotFileUrl (const QUrl& url, const QStringList& id);
		void handleGotNewItem (const QList<QStandardItem*>& item,
				const QStringList& parentId);
		void flCopyURL ();
		void flDelete ();
		void flMoveToTrash ();
		void flRestoreFromTrash ();
		void flEmptyTrash ();
		void flCreateDir ();
		void flUploadInCurrentDir ();
		void flDownload ();
		void on_AccountsBox__activated (int);
		void on_Update__released ();
		void on_Upload__released ();
		void handleContextMenuRequested (const QPoint& point);
		void handleCopiedItem (const QStringList& itemId,
				const QStringList& newParentId);
		void handleMovedItem (const QStringList& itemId,
				const QStringList& newParentId);
		void handleRestoredFromTrash (const QStringList& id);
		void handleTrashedItem (const QStringList& id);

	signals:
		void removeTab (QWidget*);

		void uploadRequested (IStorageAccount *isa, const QString& file,
				const QStringList& parentId = QStringList ());

		void gotEntity (LeechCraft::Entity entity);
	};
}
}

#endif

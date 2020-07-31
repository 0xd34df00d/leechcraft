/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETSTOREMANAGER_MANAGERTAB_H
#define PLUGINS_NETSTOREMANAGER_MANAGERTAB_H
#include <functional>
#include <QWidget>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/ihavetabs.h>
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "ui_managertab.h"

class QToolButton;
class QComboBox;
class QStandardItem;
class QAction;

namespace LC
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
	class DownManager;

	enum Columns
	{
		CName,
		CSize,
		CModify
	};

	enum SortRoles
	{
		SRName = Qt::UserRole + 1,
		SRSize,
		SRModifyDate
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

		QComboBox *AccountsBox_ = nullptr;

		QAction *Refresh_;
		QAction *Upload_;

		QHash<QByteArray, StorageItem> Id2Item_;

		enum class TransferOperation
		{
			Copy,
			Move
		};
		QPair<TransferOperation, QList<QByteArray>> TransferedIDs_;

		QAction *OpenFile_;
		QAction *CopyURL_;
		QAction *Copy_;
		QAction *Move_;
		QAction *Rename_;
		QAction *Paste_;
		QAction *DeleteFile_;
		QAction *MoveToTrash_;
		QAction *UntrashFile_;
		QAction *EmptyTrash_;
		QAction *CreateDir_;
		QAction *UploadInCurrentDir_;
		QAction *Download_;
		QAction *OpenTrash_;
		QToolButton *Trash_;

		QByteArray LastParentID_;

		QHash<IStorageAccount*, QHash<QByteArray, bool>> Account2ItemExpandState_;

		DownManager * const DownManager_;

	public:
		ManagerTab (const TabClassInfo&, AccountsManager*, ICoreProxy_ptr, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void FillToolbar ();
		void AppendAccount (IStorageAccount *acc);
		void ShowAccountActions (bool show);
		IStorageAccount* GetCurrentAccount () const;

		void ClearModel ();
		void FillListModel ();

		void RequestFileListings (IStorageAccount *acc);
		void RequestFileChanges (IStorageAccount *acc);

		QList<QByteArray> GetTrashedFiles () const;
		QList<QByteArray> GetSelectedIDs () const;
		QByteArray GetParentIDInListViewMode () const;
		QByteArray GetCurrentID () const;

		void CallOnSelection (std::function<void (ISupportFileListings *sfl, QList<QByteArray> ids)>);
		quint64 GetFolderSize (const QByteArray& id) const;
		void ShowListItemsWithParent (const QByteArray& parentId = QByteArray (),
				bool inTrash = false);

	private slots:
		void handleRefresh ();
		void handleUpload ();

		void handleDoubleClicked (const QModelIndex& idx);

		void handleAccountAdded (QObject *accObj);
		void handleAccountRemoved (QObject *accObj);

		void handleListingUpdated (const QByteArray& parentId);
		void handleGotNewItem (const StorageItem& item, const QByteArray& parentId);

		void handleFilesViewSectionResized (int index, int oldSize, int newSize);

		void performCopy (const QList<QByteArray>& ids,
				const QByteArray& parentId);
		void performMove (const QList<QByteArray>& ids,
				const QByteArray& parentId);
		void performRestoreFromTrash (const QList<QByteArray>& ids);
		void performMoveToTrash (const QList<QByteArray>& ids);
		void handleReturnPressed ();
		void handleBackspacePressed ();
		void handleQuoteLeftPressed ();

		void flOpenFile ();
		void flCopy ();
		void flMove ();
		void flRename ();
		void flPaste ();
		void flDelete ();
		void flMoveToTrash ();
		void flRestoreFromTrash ();
		void flEmptyTrash ();
		void flCreateDir ();
		void flUploadInCurrentDir ();
		void flDownload ();
		void flCopyUrl ();

		void showTrashContent (bool show);

		void handleContextMenuRequested (const QPoint& point);
		void handleExportMenuTriggered (QAction *action);

		void handleCurrentIndexChanged (int index);

		void handleGotChanges (const QList<Change>& changes);

		void handleFilterTextChanged (const QString& text);

	signals:
		void removeTab (QWidget*);

		void uploadRequested (IStorageAccount *acc, const QString& fileName,
				const QByteArray& parentId = QByteArray (), bool byHand = true);
	};
}
}

#endif

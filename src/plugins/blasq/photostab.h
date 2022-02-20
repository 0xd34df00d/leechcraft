/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <QWidget>
#include <QModelIndex>
#include <QToolBar>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/core/icoreproxy.h>
#include "ui_photostab.h"

class QComboBox;
class QSlider;

class QQuickWidget;

namespace LC
{
namespace Blasq
{
	class AccountsManager;
	class PhotosProxyModel;
	class IAccount;
	class UploadPhotosDialog;

	class PhotosTab : public QWidget
					, public ITabWidget
					, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::PhotosTab Ui_;
		QQuickWidget * const ImagesView_;

		const TabClassInfo TC_;
		QObject * const Plugin_;

		AccountsManager * const AccMgr_;
		const ICoreProxy_ptr Proxy_;

		PhotosProxyModel * const ProxyModel_;

		QComboBox *AccountsBox_;
		QAction *UploadAction_;
		QSlider *UniSlider_;
		std::unique_ptr<QToolBar> Toolbar_;

		IAccount *CurAcc_ = 0;
		QObject *CurAccObj_ = 0;

		QString OnUpdateCollectionId_;

		QString SelectedID_;
		QString SelectedCollection_;

		QStringList SelectedIDsSet_;

		bool SingleImageMode_ = false;
	public:
		PhotosTab (AccountsManager*, const TabClassInfo&, QObject*, ICoreProxy_ptr);
		PhotosTab (AccountsManager*, ICoreProxy_ptr);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		QString GetTabRecoverName () const;
		QIcon GetTabRecoverIcon () const;
		QByteArray GetTabRecoverData () const;

		void RecoverState (QDataStream&);

		void SelectAccount (const QByteArray&);

		QModelIndexList GetSelectedImages () const;
	private:
		void AddScaleSlider ();

		void HandleImageSelected (const QModelIndex&);
		void HandleCollectionSelected (const QModelIndex&);

		QModelIndex ImageID2Index (const QString&) const;
		QModelIndexList ImageID2Indexes (const QString&) const;

		QByteArray GetUniSettingName () const;

		void FinishUploadDialog (UploadPhotosDialog*);

		void PerformCtxMenu (std::function<void (QModelIndex)>);
	private slots:
		void handleAccountChosen (int);
		void handleRowChanged (const QModelIndex&);

		void on_CollectionsTree__customContextMenuRequested (const QPoint&);

		void handleScaleSlider (int);

		void uploadPhotos ();
		void handleUploadRequested ();

		void handleImageSelected (const QString&);
		void handleToggleSelectionSet (const QString&);
		void handleImageOpenRequested (const QVariant&);
		void handleImageOpenRequested ();
		void handleImageDownloadRequested (const QVariant&);
		void handleImageDownloadRequested ();
		void handleCopyURLRequested (const QVariant&);
		void handleCopyURLRequested ();
		void handleDeleteRequested (const QString&);
		void handleDeleteRequested ();
		void handleAlbumSelected (const QVariant&);
		void handleSingleImageMode (bool);

		void handleAccDoneUpdating ();
	signals:
		void removeTab ();

		void tabRecoverDataChanged ();
	};
}
}

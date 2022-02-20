/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "photostab.h"
#include <array>
#include <cmath>
#include <QToolBar>
#include <QComboBox>
#include <QMenu>
#include <QQuickWidget>
#include <QQmlNetworkAccessManagerFactory>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QClipboard>
#include <QDesktopWidget>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/standardnamfactory.h>
#include <util/sys/paths.h>
#include <util/xpc/util.h>
#include <util/network/networkdiskcache.h>
#include <util/sll/slotclosure.h>
#include <util/sll/qtutil.h>
#include "interfaces/blasq/iaccount.h"
#include "interfaces/blasq/isupportuploads.h"
#include "interfaces/blasq/isupportdeletes.h"
#include "accountsmanager.h"
#include "xmlsettingsmanager.h"
#include "uploadphotosdialog.h"
#include "photosproxymodel.h"

Q_DECLARE_METATYPE (QModelIndex)

namespace LC
{
namespace Blasq
{
	namespace
	{
		const std::array<int, 13> Zooms { { 10, 25, 33, 50, 66, 100, 150, 200, 250, 500, 750, 1000, 1600 } };
	}

	PhotosTab::PhotosTab (AccountsManager *accMgr, const TabClassInfo& tc, QObject *plugin, ICoreProxy_ptr proxy)
	: ImagesView_ (new QQuickWidget)
	, TC_ (tc)
	, Plugin_ (plugin)
	, AccMgr_ (accMgr)
	, Proxy_ (proxy)
	, ProxyModel_ (new PhotosProxyModel (this))
	, AccountsBox_ (new QComboBox)
	, Toolbar_ (new QToolBar)
	{
		Ui_.setupUi (this);
		Ui_.ImagesViewContainer_->layout ()->addWidget (ImagesView_);

		ImagesView_->setResizeMode (QQuickWidget::SizeRootObjectToView);

		auto rootCtx = ImagesView_->rootContext ();
		rootCtx->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		rootCtx->setContextProperty ("collectionModel",
				QVariant::fromValue<QObject*> (ProxyModel_));
		rootCtx->setContextProperty ("listingMode", "false");
		rootCtx->setContextProperty ("collRootIndex", QVariant::fromValue (QModelIndex ()));
		rootCtx->setContextProperty ("imageSelectionMode", tc.TabClass_.isEmpty ());

		auto engine = ImagesView_->engine ();
		engine->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine->addImportPath (cand);
		new Util::StandardNAMFactory ("blasq/qml",
			[]
			{
				return XmlSettingsManager::Instance ()
						.property ("CacheSize").toInt () * 1024 * 1024;
			},
			engine);

		const auto& path = Util::GetSysPath (Util::SysPath::QML, "blasq", "PhotoView.qml");
		ImagesView_->setSource (QUrl::fromLocalFile (path));

		auto rootObj = ImagesView_->rootObject ();
		connect (rootObj,
				SIGNAL (imageSelected (QString)),
				this,
				SLOT (handleImageSelected (QString)));
		connect (rootObj,
				SIGNAL (toggleSelectionSet (QString)),
				this,
				SLOT (handleToggleSelectionSet (QString)));
		connect (rootObj,
				SIGNAL (imageOpenRequested (QVariant)),
				this,
				SLOT (handleImageOpenRequested (QVariant)));
		connect (rootObj,
				SIGNAL (imageDownloadRequested (QVariant)),
				this,
				SLOT (handleImageDownloadRequested (QVariant)));
		connect (rootObj,
				SIGNAL (copyURLRequested (QVariant)),
				this,
				SLOT (handleCopyURLRequested (QVariant)));
		connect (rootObj,
				SIGNAL (deleteRequested (QString)),
				this,
				SLOT (handleDeleteRequested (QString)));
		connect (rootObj,
				SIGNAL (albumSelected (QVariant)),
				this,
				SLOT (handleAlbumSelected (QVariant)));
		connect (rootObj,
				SIGNAL (singleImageMode (bool)),
				this,
				SLOT (handleSingleImageMode (bool)));

		AccountsBox_->setModel (AccMgr_->GetModel ());
		AccountsBox_->setModelColumn (AccountsManager::Column::Name);
		connect (AccountsBox_,
				SIGNAL (activated (int)),
				this,
				SLOT (handleAccountChosen (int)));

		UploadAction_ = new QAction (tr ("Upload photos..."), this);
		UploadAction_->setProperty ("ActionIcon", "svn-commit");
		connect (UploadAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (uploadPhotos ()));

		Toolbar_->addWidget (AccountsBox_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (UploadAction_);

		AddScaleSlider ();

		if (AccountsBox_->count ())
			handleAccountChosen (0);

		connect (Ui_.CollectionsTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleRowChanged (QModelIndex)));
	}

	PhotosTab::PhotosTab (AccountsManager *accMgr, ICoreProxy_ptr proxy)
	: PhotosTab (accMgr, {}, nullptr, proxy)
	{
	}

	TabClassInfo PhotosTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* PhotosTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void PhotosTab::Remove ()
	{
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* PhotosTab::GetToolBar () const
	{
		return Toolbar_.get ();
	}

	QString PhotosTab::GetTabRecoverName () const
	{
		return CurAcc_ ? ("Blasq: " + CurAcc_->GetName ()) : "Blasq";
	}

	QIcon PhotosTab::GetTabRecoverIcon () const
	{
		return GetTabClassInfo ().Icon_;
	}

	QByteArray PhotosTab::GetTabRecoverData () const
	{
		QByteArray ba;
		QDataStream out (&ba, QIODevice::WriteOnly);
		out << static_cast<quint8> (1)
				<< GetTabClassInfo ().TabClass_;

		out << (CurAcc_ ? CurAcc_->GetID () : QByteArray ());
		out << SelectedCollection_;

		return ba;
	}

	void PhotosTab::RecoverState (QDataStream& in)
	{
		QByteArray accId;
		in >> accId
				>> OnUpdateCollectionId_;

		const auto accIdx = AccMgr_->GetAccountIndex (accId);
		if (accIdx < 0)
			return;

		AccountsBox_->setCurrentIndex (accIdx);
		handleAccountChosen (accIdx);
	}

	void PhotosTab::SelectAccount (const QByteArray& id)
	{
		for (int i = 0; i < AccountsBox_->count (); ++i)
			if (id == AccountsBox_->itemData (i, AccountsManager::Role::AccountId).toByteArray ())
			{
				handleAccountChosen (i);
				return;
			}
	}

	QModelIndexList PhotosTab::GetSelectedImages () const
	{
		QModelIndexList result;
		for (const auto& id : SelectedIDsSet_)
			result << ImageID2Index (id);

		if (!SelectedID_.isEmpty () && !SelectedIDsSet_.contains (SelectedID_))
			result << ImageID2Index (SelectedID_);

		return result;
	}

	void PhotosTab::AddScaleSlider ()
	{
		auto widget = new QWidget ();
		auto lay = new QHBoxLayout;
		widget->setLayout (lay);

		UniSlider_ = new QSlider (Qt::Horizontal);
		UniSlider_->setMinimumWidth (300);
		UniSlider_->setMaximumWidth (300);
		UniSlider_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
		lay->addStretch ();
		lay->addWidget (UniSlider_, 0, Qt::AlignRight);

		UniSlider_->setValue (XmlSettingsManager::Instance ()
				.Property ("ScaleSliderValue", 20).toInt ());
		connect (UniSlider_,
				SIGNAL (valueChanged (int)),
				this,
				SLOT (handleScaleSlider (int)));
		handleSingleImageMode (true);
		handleSingleImageMode (false);

		Toolbar_->addWidget (widget);
	}

	void PhotosTab::HandleImageSelected (const QModelIndex& index)
	{
		ImagesView_->rootContext ()->setContextProperty ("listingMode", QVariant (false));

		handleImageSelected (index.data (CollectionRole::ID).toString ());

		QMetaObject::invokeMethod (ImagesView_->rootObject (),
				"showImage",
				Q_ARG (QVariant, index.data (CollectionRole::Original).toUrl ()));
	}

	void PhotosTab::HandleCollectionSelected (const QModelIndex& index)
	{
		auto rootCtx = ImagesView_->rootContext ();
		if (!rootCtx->contextProperty ("listingMode").toBool ())
		{
			QMetaObject::invokeMethod (ImagesView_->rootObject (),
					"showImage",
					Q_ARG (QVariant, QUrl ()));

			rootCtx->setContextProperty ("listingMode", true);
		}

		rootCtx->setContextProperty ("collRootIndex", QVariant::fromValue (index));

		SelectedID_.clear ();
		SelectedCollection_ = index.data (CollectionRole::ID).toString ();

		emit tabRecoverDataChanged ();
	}

	QModelIndex PhotosTab::ImageID2Index (const QString& id) const
	{
		auto model = CurAcc_->GetCollectionsModel ();
		QModelIndex allPhotosIdx;
		for (auto i = 0; i < model->rowCount (); ++i)
		{
			const auto& idx = model->index (i, 0);
			if (idx.data (CollectionRole::Type).toInt () == ItemType::AllPhotos)
			{
				allPhotosIdx = idx;
				break;
			}
		}

		if (!allPhotosIdx.isValid ())
			return {};

		for (auto i = 0, rc = model->rowCount (allPhotosIdx); i < rc; ++i)
		{
			const auto& idx = model->index (i, 0, allPhotosIdx);
			if (idx.data (CollectionRole::ID).toString () == id)
				return idx;
		}

		return {};
	}

	namespace
	{
		QModelIndexList ScanIndex (const QString& id, const QModelIndex& parent, QAbstractItemModel * const model)
		{
			QModelIndexList result;

			for (auto i = 0; i < model->rowCount (parent); ++i)
			{
				const auto& idx = model->index (i, 0, parent);
				if (idx.data (CollectionRole::Type).toInt () != ItemType::Image)
					result += ScanIndex (id, idx, model);
				else if (idx.data (CollectionRole::ID).toString () == id)
					result += idx;
			}

			return result;
		}
	}

	QModelIndexList PhotosTab::ImageID2Indexes (const QString& id) const
	{
		return ScanIndex (id, {}, CurAcc_->GetCollectionsModel ());
	}

	QByteArray PhotosTab::GetUniSettingName () const
	{
		return SingleImageMode_ ? "ZoomSliderValue" : "ScaleSliderValue";
	}

	void PhotosTab::FinishUploadDialog (UploadPhotosDialog *dia)
	{
		dia->show ();
		dia->setAttribute (Qt::WA_DeleteOnClose);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, dia]
			{
				auto isu = qobject_cast<ISupportUploads*> (CurAccObj_);
				isu->UploadImages (dia->GetSelectedCollection (), dia->GetSelectedFiles ());
			},
			dia,
			SIGNAL (accepted ()),
			dia
		};
	}

	void PhotosTab::PerformCtxMenu (std::function<void (QModelIndex)> functor)
	{
		const auto& idx = sender ()->property ("Blasq/Index").value<QModelIndex> ();
		if (!idx.isValid ())
			return;

		auto rows = Ui_.CollectionsTree_->selectionModel ()->selectedRows ();
		if (!rows.contains (idx))
			rows.prepend (idx);

		for (const auto& row : rows)
			functor (row);
	}

	void PhotosTab::handleAccountChosen (int idx)
	{
		auto accVar = AccountsBox_->itemData (idx, AccountsManager::Role::AccountObj);
		auto accObj = accVar.value<QObject*> ();
		auto acc = qobject_cast<IAccount*> (accObj);
		if (acc == CurAcc_)
			return;

		if (CurAccObj_)
			disconnect (CurAccObj_,
					0,
					this,
					0);

		CurAccObj_ = accObj;
		CurAcc_ = acc;

		connect (CurAccObj_,
				SIGNAL (doneUpdating ()),
				this,
				SLOT (handleAccDoneUpdating ()));

		CurAcc_->UpdateCollections ();

		auto model = CurAcc_->GetCollectionsModel ();

		if (auto sel = Ui_.CollectionsTree_->selectionModel ())
			disconnect (sel,
					SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
					this,
					SLOT (handleRowChanged (QModelIndex)));
		Ui_.CollectionsTree_->setModel (model);
		connect (Ui_.CollectionsTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleRowChanged (QModelIndex)));

		ProxyModel_->SetCurrentAccount (CurAccObj_);
		ProxyModel_->setSourceModel (model);

		ImagesView_->rootContext ()->setContextProperty ("collRootIndex", QVariant::fromValue (QModelIndex ()));
		HandleCollectionSelected ({});

		UploadAction_->setEnabled (qobject_cast<ISupportUploads*> (CurAccObj_));

		emit tabRecoverDataChanged ();
	}

	void PhotosTab::handleRowChanged (const QModelIndex& index)
	{
		if (index.data (CollectionRole::Type).toInt () == ItemType::Image)
			HandleImageSelected (ProxyModel_->mapFromSource (index));
		else
			HandleCollectionSelected (ProxyModel_->mapFromSource (index));
	}

	void PhotosTab::on_CollectionsTree__customContextMenuRequested (const QPoint& point)
	{
		const auto& idx = Ui_.CollectionsTree_->indexAt (point);
		if (!idx.isValid ())
			return;

		const auto type = idx.data (CollectionRole::Type).toInt ();
		const auto isAll = type == ItemType::AllPhotos;
		const auto isColl = type == ItemType::Collection;
		const auto isImage = type == ItemType::Image;

		QMenu menu;
		if (isImage)
		{
			menu.addAction (Proxy_->GetIconThemeManager ()->GetIcon ("go-jump-locationbar"),
					tr ("Open in browser"),
					this,
					SLOT (handleImageOpenRequested ()));
			menu.addAction (Proxy_->GetIconThemeManager ()->GetIcon ("download"),
					tr ("Download original"),
					this,
					SLOT (handleImageDownloadRequested ()));
			menu.addAction (Proxy_->GetIconThemeManager ()->GetIcon ("edit-copy"),
					tr ("Copy image URL"),
					this,
					SLOT (handleCopyURLRequested ()));
		}

		if (auto isd = qobject_cast<ISupportDeletes*> (CurAccObj_))
		{
			if ((isColl && isd->SupportsFeature (DeleteFeature::DeleteCollections)) ||
				(isImage && isd->SupportsFeature (DeleteFeature::DeleteImages)))
				menu.addAction (Proxy_->GetIconThemeManager ()->GetIcon ("list-remove"),
						tr ("Delete"),
						this,
						SLOT (handleDeleteRequested ()));
		}

		if (auto isu = qobject_cast<ISupportUploads*> (CurAccObj_))
			if (isColl || (isAll && !isu->HasUploadFeature (ISupportUploads::Feature::RequiresAlbumOnUpload)))
				menu.addAction (Proxy_->GetIconThemeManager ()->GetIcon ("svn-commit"),
						tr ("Upload"),
						this,
						SLOT (handleUploadRequested ()));

		const auto idxVar = QVariant::fromValue (idx);
		for (auto act : menu.actions ())
			act->setProperty ("Blasq/Index", idxVar);

		if (!menu.actions ().isEmpty ())
			menu.exec (Ui_.CollectionsTree_->viewport ()->mapToGlobal (point));
	}

	void PhotosTab::handleScaleSlider (int value)
	{
		if (SingleImageMode_)
			ImagesView_->rootObject ()->setProperty ("imageZoom", Zooms [value]);
		else
		{
			const auto width = qApp->desktop ()->screenGeometry (this).width ();
			const int lowest = width / 20.;
			const int highest = width / 5.;

			// value from 0 to 100; at 0 it should be lowest, at 100 it should be highest
			const auto computed = (highest - lowest) / (std::exp (1) - 1) * (std::exp (value / 100.) - 1) + lowest;
			ImagesView_->rootObject ()->setProperty ("cellSize", computed);
		}
		XmlSettingsManager::Instance ().setProperty (GetUniSettingName (), value);
	}

	void PhotosTab::uploadPhotos ()
	{
		const auto dia = new UploadPhotosDialog { CurAccObj_, this };

		auto curSelectedIdx = Ui_.CollectionsTree_->currentIndex ();
		if (curSelectedIdx.data (CollectionRole::Type).toInt () == ItemType::Image)
			curSelectedIdx = curSelectedIdx.parent ();
		if (curSelectedIdx.data (CollectionRole::Type).toInt () == ItemType::Collection)
			dia->SetSelectedCollection (curSelectedIdx);

		FinishUploadDialog (dia);
	}

	void PhotosTab::handleUploadRequested ()
	{
		PerformCtxMenu ([this] (const QModelIndex& idx)
				{
					const auto dia = new UploadPhotosDialog (CurAccObj_, this);
					dia->SetSelectedCollection (idx);
					FinishUploadDialog (dia);
				});
	}

	void PhotosTab::handleImageSelected (const QString& id)
	{
		SelectedID_ = id;
	}

	void PhotosTab::handleToggleSelectionSet (const QString& id)
	{
		const auto& idxs = ImageID2Indexes (id);

		if (SelectedIDsSet_.removeAll (id))
			ProxyModel_->RemoveSelected (id, idxs);
		else
		{
			SelectedIDsSet_ << id;
			ProxyModel_->AddSelected (id, idxs);
		}
	}

	void PhotosTab::handleImageOpenRequested (const QVariant& var)
	{
		const auto& url = var.toUrl ();
		if (!url.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid URL"
					<< var;
			return;
		}

		const auto& entity = Util::MakeEntity (url, QString (), FromUserInitiated | OnlyHandle);
		Proxy_->GetEntityManager ()->HandleEntity (entity);
	}

	void PhotosTab::handleImageOpenRequested ()
	{
		PerformCtxMenu ([this] (const QModelIndex& idx) -> void
				{
					const auto& url = idx.data (CollectionRole::Original).toUrl ();
					const auto& entity = Util::MakeEntity (url, QString (), FromUserInitiated | OnlyHandle);
					Proxy_->GetEntityManager ()->HandleEntity (entity);
				});
	}

	void PhotosTab::handleImageDownloadRequested (const QVariant& var)
	{
		const auto& url = var.toUrl ();
		if (!url.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid URL"
					<< var;
			return;
		}

		const auto& entity = Util::MakeEntity (url, QString (), FromUserInitiated | OnlyDownload);
		Proxy_->GetEntityManager ()->HandleEntity (entity);
	}

	void PhotosTab::handleImageDownloadRequested ()
	{
		PerformCtxMenu ([this] (const QModelIndex& idx) -> void
				{
					const auto& url = idx.data (CollectionRole::Original).toUrl ();
					const auto& entity = Util::MakeEntity (url, QString (), FromUserInitiated | OnlyDownload);
					Proxy_->GetEntityManager ()->HandleEntity (entity);
				});
	}

	void PhotosTab::handleCopyURLRequested (const QVariant& var)
	{
		const auto& url = var.toUrl ();
		if (!url.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid URL"
					<< var;
			return;
		}

		auto cb = qApp->clipboard ();
		cb->setText (url.toString (), QClipboard::Clipboard);
	}

	void PhotosTab::handleCopyURLRequested ()
	{
		auto cb = qApp->clipboard ();
		PerformCtxMenu ([cb] (const QModelIndex& idx) -> void
				{
					const auto& url = idx.data (CollectionRole::Original).toUrl ();
					cb->setText (url.toString (), QClipboard::Clipboard);
				});
	}

	void PhotosTab::handleDeleteRequested (const QString& id)
	{
		auto isd = qobject_cast<ISupportDeletes*> (CurAccObj_);
		if (!isd)
			return;

		const auto& idx = ImageID2Index (id);
		if (idx.isValid ())
			isd->Delete (idx);
	}

	void PhotosTab::handleDeleteRequested ()
	{
		auto isd = qobject_cast<ISupportDeletes*> (CurAccObj_);
		if (!isd)
			return;

		const auto& idx = sender ()->property ("Blasq/Index").value<QModelIndex> ();
		if (idx.isValid ())
			isd->Delete (idx);
	}

	void PhotosTab::handleAlbumSelected (const QVariant& var)
	{
		const auto& index = var.value<QModelIndex> ();
		Ui_.CollectionsTree_->setCurrentIndex (ProxyModel_->mapToSource (index));
	}

	void PhotosTab::handleSingleImageMode (bool single)
	{
		SingleImageMode_ = single;

		const auto defValue = SingleImageMode_ ?
				std::distance (Zooms.begin (), std::find (Zooms.begin (), Zooms.end (), 100)) :
				20;

		const auto value = XmlSettingsManager::Instance ()
				.Property (Util::AsStringView (GetUniSettingName ()), static_cast<int> (defValue)).toInt ();
		if (value > UniSlider_->maximum ())
		{
			UniSlider_->setRange (0, SingleImageMode_ ? Zooms.size () - 1 : 100);
			UniSlider_->setValue (value);
		}
		else
		{
			UniSlider_->setValue (value);
			UniSlider_->setRange (0, SingleImageMode_ ? Zooms.size () - 1 : 100);
		}
	}

	namespace
	{
		QModelIndex FindCollection (QAbstractItemModel *model, const QModelIndex& parent, const QString& id)
		{
			for (int i = 0, rc = model->rowCount (parent); i < rc; ++i)
			{
				const auto& idx = model->index (i, 0, parent);
				if (idx.data (CollectionRole::Type).toInt () != ItemType::Collection)
					continue;

				if (idx.data (CollectionRole::ID).toString () == id)
					return idx;

				const auto& child = FindCollection (model, idx, id);
				if (child.isValid ())
					return child;
			}

			return {};
		}
	}

	void PhotosTab::handleAccDoneUpdating ()
	{
		if (OnUpdateCollectionId_.isEmpty () || !CurAcc_)
			return;

		const auto& idx = FindCollection (CurAcc_->GetCollectionsModel (), {}, OnUpdateCollectionId_);
		OnUpdateCollectionId_.clear ();
		if (!idx.isValid ())
			return;

		Ui_.CollectionsTree_->setCurrentIndex (idx);
	}
}
}

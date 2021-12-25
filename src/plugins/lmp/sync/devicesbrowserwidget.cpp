/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "devicesbrowserwidget.h"
#include <algorithm>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <util/models/flattenfiltermodel.h>
#include <util/models/mergemodel.h>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <interfaces/devices/iremovabledevmanager.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/lmp/isyncplugin.h>
#include <interfaces/lmp/iunmountablesync.h>
#include "core.h"
#include "localcollection.h"
#include "localcollectionmodel.h"
#include "uploadmodel.h"
#include "syncmanager.h"
#include "transcodingparams.h"
#include "unmountabledevmanager.h"
#include "syncunmountablemanager.h"
#include "collectionsmanager.h"

typedef QMap<QString, LC::LMP::TranscodingParams> TranscodingParamsMap_t;
Q_DECLARE_METATYPE (TranscodingParamsMap_t)

namespace LC
{
namespace LMP
{
	namespace
	{
		class MountableFlattener : public Util::FlattenFilterModel
		{
		public:
			MountableFlattener (QObject *parent)
			: Util::FlattenFilterModel (parent)
			{
			}

			QVariant data (const QModelIndex& index, int role) const
			{
				if (role != Qt::DisplayRole)
					return Util::FlattenFilterModel::data (index, role);

				const auto& mounts = index.data (MassStorageRole::MountPoints).toStringList ();
				const auto& mountText = mounts.isEmpty () ?
						DevicesBrowserWidget::tr ("not mounted") :
						DevicesBrowserWidget::tr ("mounted at %1").arg (mounts.join ("; "));

				const auto& size = index.data (MassStorageRole::TotalSize).toLongLong ();
				return QString ("%1 (%2, %3), %4")
						.arg (index.data (MassStorageRole::VisibleName).toString ())
						.arg (Util::MakePrettySize (size))
						.arg (index.data (MassStorageRole::DevFile).toString ())
						.arg (mountText);
			}
		protected:
			bool IsIndexAccepted (const QModelIndex& child) const
			{
				return child.data (MassStorageRole::IsMountable).toBool ();
			}
		};
	}

	DevicesBrowserWidget::DevicesBrowserWidget (QWidget *parent)
	: QWidget (parent)
	, DevUploadModel_ (new UploadModel (this))
	, Merger_ (new Util::MergeModel (QStringList ("Device name"), this))
	, UnmountableMgr_ (new UnmountableDevManager (this))
	{
		LoadLastParams ();

		Ui_.setupUi (this);

		DevUploadModel_->setSourceModel (Core::Instance ().GetCollectionsManager ()->GetModel ());
		Ui_.OurCollection_->setModel (DevUploadModel_);

		auto connectManager = [this] (SyncManagerBase *manager) -> void
		{
			connect (manager,
					SIGNAL (uploadLog (QString)),
					this,
					SLOT (appendUpLog (QString)));

			connect (manager,
					SIGNAL (transcodingProgress (int, int, SyncManagerBase*)),
					this,
					SLOT (handleTranscodingProgress (int, int)));
			connect (manager,
					SIGNAL (uploadProgress (int, int, SyncManagerBase*)),
					this,
					SLOT (handleUploadProgress (int, int)));
			connect (manager,
					SIGNAL (singleUploadProgress (int, int, SyncManagerBase*)),
					this,
					SLOT (handleSingleUploadProgress (int, int)));
		};

		connectManager (Core::Instance ().GetSyncManager ());
		connectManager (Core::Instance ().GetSyncUnmountableManager ());

		Ui_.TSProgress_->hide ();
		Ui_.UploadProgress_->hide ();
		Ui_.SingleUploadProgress_->hide ();

		Ui_.UnmountablePartsWidget_->hide ();
	}

	void DevicesBrowserWidget::InitializeDevices ()
	{
		auto pm = GetProxyHolder ()->GetPluginsManager ();

		const auto& mgrs = pm->GetAllCastableTo<IRemovableDevManager*> ();
		for (const auto& mgr : mgrs)
		{
			if (!mgr->SupportsDevType (DeviceType::MassStorage))
				continue;

			auto flattener = new MountableFlattener (this);
			flattener->SetSource (mgr->GetDevicesModel ());
			Merger_->AddModel (flattener);
			Flattener2DevMgr_ [flattener] = mgr;
		}

		UnmountableMgr_->InitializePlugins ();
		Merger_->AddModel (UnmountableMgr_->GetDevListModel ());

		Ui_.DevicesSelector_->setModel (Merger_);
		connect (Merger_,
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleDevDataChanged (QModelIndex, QModelIndex)));
		connect (Merger_,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsInserted (QModelIndex, int, int)));

		for (int i = 0; i < Ui_.DevicesSelector_->count (); ++i)
		{
			const auto& thatId = Ui_.DevicesSelector_->
					itemData (i, CommonDevRole::DevPersistentID).toString ();
			if (thatId != LastDevice_)
				continue;

			Ui_.DevicesSelector_->setCurrentIndex (i);
			on_DevicesSelector__activated (i);
			break;
		}
	}

	void DevicesBrowserWidget::LoadLastParams ()
	{
		qRegisterMetaType<TranscodingParams> ("TranscodingParams");
		qRegisterMetaTypeStreamOperators<TranscodingParams> ();
		qRegisterMetaType<QMap<QString, TranscodingParams>> ("QMap<QString, TranscodingParams>");
		qRegisterMetaTypeStreamOperators<QMap<QString, TranscodingParams>> ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Transcoding");
		settings.beginGroup ("Transcoding");
		Device2Params_ = settings.value ("LastParams").value<decltype (Device2Params_)> ();
		LastDevice_ = settings.value ("LastDeviceID").toString ();
		settings.endGroup ();
	}

	void DevicesBrowserWidget::SaveLastParams () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Transcoding");
		settings.beginGroup ("Transcoding");
		settings.setValue ("LastParams", QVariant::fromValue (Device2Params_));

		const auto idx = Ui_.DevicesSelector_->currentIndex ();
		const auto& devId = idx >= 0 ?
				Ui_.DevicesSelector_->itemData (idx, CommonDevRole::DevPersistentID).toString () :
				QString ();
		settings.setValue ("LastDeviceID", devId);

		settings.endGroup ();
	}

	namespace
	{
		QList<ISyncPlugin*> FindSuitables (const QString& mountPath)
		{
			QList<ISyncPlugin*> suitables;
			for (auto syncer : Core::Instance ().GetSyncPlugins ())
			{
				auto isp = qobject_cast<ISyncPlugin*> (syncer);
				if (isp->CouldSync (mountPath) != SyncConfLevel::None)
					suitables << isp;
			}
			return suitables;
		}
	}

	void DevicesBrowserWidget::UploadMountable (int idx)
	{
		const auto& to = Ui_.DevicesSelector_->
				itemData (idx, MassStorageRole::MountPoints).toStringList ().value (0);
		if (to.isEmpty ())
			return;

		const auto& suitables = FindSuitables (to);
		if (suitables.size () == 1)
			CurrentSyncer_ = suitables.value (0);
		else
		{
			auto items = Util::Map (suitables, &ISyncPlugin::GetSyncSystemName);

			const auto& name = QInputDialog::getItem (this,
					tr ("Select syncer"),
					tr ("Multiple different syncers can handle the device %1, what do you want to use?")
						.arg (Ui_.DevicesSelector_->itemText (idx)),
					items);
			if (name.isEmpty ())
				return;

			CurrentSyncer_ = suitables.value (items.indexOf (name));
		}

		auto paths = Util::Map (DevUploadModel_->GetSelectedIndexes ().values (),
				[] (const QModelIndex& idx) { return idx.data (LocalCollectionModel::Role::TrackPath).toString (); });
		paths.removeAll ({});

		Ui_.UploadLog_->clear ();

		const auto& params = Ui_.TranscodingOpts_->GetParams ();
		Core::Instance ().GetSyncManager ()->AddFiles (CurrentSyncer_, to, paths, params);
	}

	void DevicesBrowserWidget::UploadUnmountable (int idx)
	{
		int starting = 0;
		Merger_->GetModelForRow (idx, &starting);
		idx -= starting;

		auto paths = Util::Map (DevUploadModel_->GetSelectedIndexes ().values (),
				[] (const QModelIndex& idx) { return idx.data (LocalCollectionModel::Role::TrackPath).toString (); });
		paths.removeAll ({});

		auto syncer = qobject_cast<IUnmountableSync*> (UnmountableMgr_->GetDeviceManager (idx));
		const auto& info = UnmountableMgr_->GetDeviceInfo (idx);

		const int partIdx = Ui_.UnmountablePartsBox_->currentIndex ();
		const auto& storageId = Ui_.UnmountablePartsBox_->itemData (partIdx).toByteArray ();
		const auto& params = Ui_.TranscodingOpts_->GetParams ();
		Core::Instance ().GetSyncUnmountableManager ()->AddFiles ({ syncer, info.ID_,
				storageId, paths, params });
	}

	void DevicesBrowserWidget::HandleMountableSelected (int idx)
	{
		Ui_.MountButton_->show ();
		Ui_.TranscodingOpts_->SetMaskVisible (true);
		Ui_.UnmountablePartsWidget_->hide ();

		auto isMounted = Ui_.DevicesSelector_->
				itemData (idx, MassStorageRole::IsMounted).toBool ();
		Ui_.MountButton_->setEnabled (!isMounted);

		if (!isMounted)
			return;

		const auto& mountPath = Ui_.DevicesSelector_->
				itemData (idx, MassStorageRole::MountPoints).toStringList ().value (0);
		if (mountPath.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "device seems to be mounted, but no mount points available:"
					<< Ui_.DevicesSelector_->itemData (idx, CommonDevRole::DevID).toString ();
			return;
		}

		Ui_.SyncTabs_->setEnabled (!FindSuitables (mountPath).isEmpty ());
	}

	void DevicesBrowserWidget::HandleUnmountableSelected (int idx)
	{
		Ui_.MountButton_->hide ();
		Ui_.TranscodingOpts_->SetMaskVisible (false);
		Ui_.UnmountablePartsWidget_->show ();

		int starting = 0;
		Merger_->GetModelForRow (idx, &starting);
		idx -= starting;

		Ui_.UnmountablePartsBox_->clear ();
		const auto& info = UnmountableMgr_->GetDeviceInfo (idx);
		for (const auto& storage : info.Partitions_)
		{
			const auto& boxText = storage.TotalSize_ > 0 ?
					tr ("%1 (%2 available of %3)")
							.arg (storage.Name_)
							.arg (Util::MakePrettySize (storage.AvailableSize_))
							.arg (Util::MakePrettySize (storage.TotalSize_)) :
					storage.Name_;
			Ui_.UnmountablePartsBox_->addItem (boxText, storage.ID_);
		}
	}

	void DevicesBrowserWidget::handleDevDataChanged (const QModelIndex& from, const QModelIndex& to)
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < from.row () && idx > to.row ())
			return;

		on_DevicesSelector__activated (idx);
	}

	void DevicesBrowserWidget::handleRowsInserted (const QModelIndex&, int start, int end)
	{
		if (start - end + 1 == Merger_->rowCount ())
			on_DevicesSelector__activated (0);
	}

	void DevicesBrowserWidget::on_UploadButton__released ()
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < 0)
			return;

		if (Flattener2DevMgr_.contains (*Merger_->GetModelForRow (idx)))
			UploadMountable (idx);
		else
			UploadUnmountable (idx);

		const auto& devId = Ui_.DevicesSelector_->
				itemData (idx, CommonDevRole::DevPersistentID).toString ();
		Device2Params_ [devId] = Ui_.TranscodingOpts_->GetParams ();
		SaveLastParams ();
	}

	void DevicesBrowserWidget::on_RefreshButton__released ()
	{
		UnmountableMgr_->Refresh ();
	}

	void DevicesBrowserWidget::on_DevicesSelector__activated (int idx)
	{
		CurrentSyncer_ = 0;

		if (idx < 0)
		{
			Ui_.MountButton_->setEnabled (false);
			Ui_.UnmountablePartsWidget_->hide ();
			return;
		}

		if (Flattener2DevMgr_.contains (*Merger_->GetModelForRow (idx)))
			HandleMountableSelected (idx);
		else
			HandleUnmountableSelected (idx);

		const auto& devId = Ui_.DevicesSelector_->
				itemData (idx, CommonDevRole::DevPersistentID).toString ();
		if (Device2Params_.contains (devId))
			Ui_.TranscodingOpts_->SetParams (Device2Params_.value (devId));
	}

	void DevicesBrowserWidget::on_MountButton__released ()
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < 0)
			return;

		const auto model = *Merger_->GetModelForRow (idx);
		if (!Flattener2DevMgr_.contains (model))
			return;

		const auto& id = Ui_.DevicesSelector_->itemData (idx, CommonDevRole::DevID).toString ();
		Flattener2DevMgr_ [model]->MountDevice (id);
	}

	void DevicesBrowserWidget::appendUpLog (QString text)
	{
		text.prepend (QTime::currentTime ().toString ("[HH:mm:ss.zzz] "));
		Ui_.UploadLog_->append ("<code>" + text + "</code>");
	}

	void DevicesBrowserWidget::handleTranscodingProgress (int done, int total)
	{
		Ui_.TSProgress_->setVisible (done < total);
		Ui_.TSProgress_->setMaximum (total);
		Ui_.TSProgress_->setValue (done);
	}

	void DevicesBrowserWidget::handleUploadProgress (int done, int total)
	{
		const auto visible = done < total;
		Ui_.UploadProgress_->setVisible (visible);
		if (!visible)
			Ui_.SingleUploadProgress_->hide ();

		Ui_.UploadProgress_->setMaximum (total);
		Ui_.UploadProgress_->setValue (done);
	}

	void DevicesBrowserWidget::handleSingleUploadProgress (int done, int total)
	{
		Ui_.SingleUploadProgress_->setVisible (done < total && total > 0);
		Ui_.SingleUploadProgress_->setMaximum (total);
		Ui_.SingleUploadProgress_->setValue (done);
	}
}
}

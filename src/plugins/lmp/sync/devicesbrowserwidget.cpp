/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "devicesbrowserwidget.h"
#include <algorithm>
#include <QConcatenateTablesProxyModel>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <util/models/flattenfiltermodel.h>
#include <util/models/mergemodel.h>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/threads/coro/context.h>
#include <util/threads/coro/task.h>
#include <interfaces/devices/iremovabledevmanager.h>
#include <interfaces/devices/deviceroles.h>
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
#include "collectionsmanager.h"

typedef QMap<QString, LC::LMP::TranscodingParams> TranscodingParamsMap_t;
Q_DECLARE_METATYPE (TranscodingParamsMap_t)

namespace LC
{
namespace LMP
{
	DevicesBrowserWidget::DevicesBrowserWidget (QWidget *parent)
	: QWidget (parent)
	, DevUploadModel_ (new UploadModel (this))
	, Merger_ { std::make_unique<QConcatenateTablesProxyModel> (this) }
	{
		LoadLastParams ();

		Ui_.setupUi (this);

		connect (Ui_.DevicesSelector_,
				&QComboBox::activated,
				this,
				&DevicesBrowserWidget::UpdateGuiForSyncer);

		DevUploadModel_->setSourceModel (Core::Instance ().GetCollectionsManager ()->GetModel ());
		Ui_.OurCollection_->setModel (DevUploadModel_);

		auto connectManager = [this] (auto manager)
		{
			/*
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
					*/
		};

		Ui_.TSProgress_->hide ();
		Ui_.UploadProgress_->hide ();
		Ui_.SingleUploadProgress_->hide ();
	}

	DevicesBrowserWidget::~DevicesBrowserWidget () = default;

	void DevicesBrowserWidget::InitializeUploaders ()
	{
		connect (&*Merger_,
				&QAbstractItemModel::rowsInserted,
				this,
				[this]
				{
					if (Ui_.DevicesSelector_->currentIndex () == -1)
					{
						Ui_.DevicesSelector_->setCurrentIndex (0);
						UpdateGuiForSyncer (0);
					}
				});

		const auto pm = GetProxyHolder ()->GetPluginsManager ();
		for (const auto& syncer : pm->GetAllCastableTo<ISyncPlugin*> ())
		{
			auto& model = syncer->GetSyncTargetsModel ();
			Merger_->addSourceModel (&model);
			Model2Syncer_ [&model] = syncer;
		}

		Ui_.DevicesSelector_->setModel (&*Merger_);
	}

	void DevicesBrowserWidget::UpdateGuiForSyncer (int idx)
	{
		if (idx < 0)
			return;

		if (const auto& devId = Ui_.DevicesSelector_->itemData (idx, CommonDevRole::DevPersistentID).toString ();
			Device2Params_.contains (devId))
			Ui_.TranscodingOpts_->SetParams (Device2Params_.value (devId));

		if (const auto syncer = GetSyncerForIndex (idx))
		{
			SyncerConfigWidget_ = syncer->MakeConfigWidget ();
			Ui_.SyncOptsLayout_->insertWidget (0, SyncerConfigWidget_->GetQWidget ());
		}
	}

	QModelIndex DevicesBrowserWidget::GetSourceIndex (int idx) const
	{
		return Merger_->mapToSource (Merger_->index (idx, 0));
	}

	ISyncPlugin* DevicesBrowserWidget::GetSyncerForIndex (int idx) const
	{
		if (idx < 0)
			return nullptr;

		const auto syncerModel = GetSourceIndex (idx).model ();
		if (const auto syncer = Model2Syncer_.value (syncerModel))
			return syncer;

		qWarning () << "unknown syncer for" << syncerModel;
		return nullptr;
	}

	void DevicesBrowserWidget::LoadLastParams ()
	{
		qRegisterMetaType<TranscodingParams> ("TranscodingParams");
		qRegisterMetaType<QMap<QString, TranscodingParams>> ("QMap<QString, TranscodingParams>");

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Transcoding");
		settings.beginGroup ("Transcoding");
		Device2Params_ = settings.value ("LastParams").value<decltype (Device2Params_)> ();
		settings.endGroup ();
	}

	void DevicesBrowserWidget::SaveLastParams () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Transcoding");
		settings.beginGroup ("Transcoding");
		settings.setValue ("LastParams", QVariant::fromValue (Device2Params_));
		settings.endGroup ();
	}

	void DevicesBrowserWidget::on_UploadButton__released ()
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < 0)
			return;

		const auto& devId = Ui_.DevicesSelector_->itemData (idx, CommonDevRole::DevPersistentID).toString ();
		Device2Params_ [devId] = Ui_.TranscodingOpts_->GetParams ();
		SaveLastParams ();

		const auto syncer = GetSyncerForIndex (idx);
		if (!syncer)
			return;

		auto paths = Util::Map (DevUploadModel_->GetSelectedIndexes ().values (),
				[] (const QModelIndex& idx) { return idx.data (LocalCollectionModel::Role::TrackPath).toString (); });
		paths.removeAll ({});

		Ui_.UploadLog_->clear ();

		[&, this] () -> Util::Task<void>
		{
			SyncManager mgr;
			co_await mgr.RunUpload (paths, Ui_.TranscodingOpts_->GetParams (),
					{
						.Syncer_ = syncer,
						.Target_ = GetSourceIndex (idx),
						.Config_ = SyncerConfigWidget_->GetConfig (),
					});
		} ();
	}

	void DevicesBrowserWidget::on_RefreshButton__released ()
	{
		// TODO
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

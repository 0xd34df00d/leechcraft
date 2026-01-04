/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "devicesbrowserwidget.h"
#include <QConcatenateTablesProxyModel>
#include <QFileInfo>
#include <QSettings>
#include <util/gui/util.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/threads/coro/context.h>
#include <util/threads/coro/task.h>
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
#include "progressmanager.h"

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

		connect (Ui_.RefreshButton_,
				&QPushButton::released,
				this,
				[this]
				{
					for (auto syncer : Model2Syncer_)
						syncer->RefreshSyncTargets ();
				});

		connect (Ui_.DevicesSelector_,
				&QComboBox::activated,
				this,
				&DevicesBrowserWidget::UpdateGuiForSyncer);

		DevUploadModel_->setSourceModel (Core::Instance ().GetCollectionsManager ()->GetModel ());
		Ui_.OurCollection_->setModel (DevUploadModel_);

		Ui_.TSProgress_->hide ();
		Ui_.UploadProgress_->hide ();
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

		Ui_.DevicesSelector_->setModel (&*Merger_);

		const auto pm = GetProxyHolder ()->GetPluginsManager ();
		for (const auto& syncer : pm->GetAllCastableTo<ISyncPlugin*> ())
		{
			auto& model = syncer->GetSyncTargetsModel ();
			Merger_->addSourceModel (&model);
			Model2Syncer_ [&model] = syncer;
		}
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

	namespace
	{
		class ProgressTracker final
		{
			Q_DECLARE_TR_FUNCTIONS (LC::LMP::ProgressManager)

			QProgressBar& TranscodingBar_;
			QProgressBar& CopyingBar_;

			ProgressManager::Handle TranscodingProgress_;
			ProgressManager::Handle CopyingProgress_;
		public:
			explicit ProgressTracker (qsizetype count, QProgressBar& transcoding, QProgressBar& copying)
			: TranscodingBar_ { transcoding }
			, CopyingBar_ { copying }
			, TranscodingProgress_ { Core::Instance ().GetProgressManager ()->Add (MakeItem (tr ("Audio transcoding"), count)) }
			, CopyingProgress_ { Core::Instance ().GetProgressManager ()->Add (MakeItem (tr ("Audio copying"), count)) }
			{
				for (const auto bar : { &transcoding, &copying })
				{
					bar->setVisible (true);
					bar->setMaximum (count);
					bar->setValue (0);
				}
			}

			~ProgressTracker ()
			{
				TranscodingBar_.setVisible (false);
				CopyingBar_.setVisible (false);
			}

			void HandleSyncEvent (const SyncEvents::Event& event)
			{
				Util::Visit (event,
						[&] (const SyncEvents::XcodingSkipped& e)
						{
							TranscodingBar_.setMaximum (TranscodingBar_.maximum () - e.Count_);
							TranscodingProgress_.SetTotal (TranscodingProgress_.GetTotal () - e.Count_);
						},
						[&] (const SyncEvents::XcodingFinished&)
						{
							TranscodingBar_.setValue (TranscodingBar_.value () + 1);
							++TranscodingProgress_;
						},
						[&] (const SyncEvents::CopyFinished&)
						{
							CopyingBar_.setValue (CopyingBar_.value () + 1);
							++CopyingProgress_;
						},
						[] (const auto&) {});
			}
		private:
			static ProgressManager::Item MakeItem (const QString& name, int count)
			{
				return { .Name_ = name, .StatusPattern_ = tr ("%1 of %2"), .Type_ = ProcessProgress, .Total_ = count };
			}
		};
	}

	void DevicesBrowserWidget::on_UploadButton__released ()
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < 0)
			return;

		const auto& modelIndex = GetSourceIndex (idx);

		const auto& devId = modelIndex.data (CommonDevRole::DevPersistentID).toString ();
		Device2Params_ [devId] = Ui_.TranscodingOpts_->GetParams ();
		SaveLastParams ();

		const auto syncer = GetSyncerForIndex (idx);
		if (!syncer)
			return;

		Ui_.UploadLog_->clear ();

		const auto& paths = DevUploadModel_->GetSelectedPaths ();
		[&, this] -> Util::ContextTask<void>
		{
			co_await Util::AddContextObject { *this };

			ProgressTracker progress { paths.size (), *Ui_.TSProgress_, *Ui_.UploadProgress_ };
			SyncManager mgr;
			connect (&mgr,
					&SyncManager::syncEvent,
					this,
					[this, &progress] (const SyncEvents::Event& event)
					{
						HandleSyncEvent (event);
						progress.HandleSyncEvent (event);
					});
			co_await mgr.RunUpload (paths, Ui_.TranscodingOpts_->GetParams (),
					{
						.Syncer_ = syncer,
						.Target_ = GetSourceIndex (idx),
						.Config_ = SyncerConfigWidget_->GetConfig (),
					});
		} ();
	}

	namespace
	{
		auto PrepareStrings (SyncEvents::Event event)
		{
			Util::Visit (event,
					[] (SyncEvents::XcodingSkipped&) {},
					[] (SyncEvents::TranscodingData& data)
					{
						data.Orig_ = Util::FormatName (QFileInfo { data.Orig_ }.fileName ());
						data.Target_ = Util::FormatName (QFileInfo { data.Target_ }.fileName ());
					},
					[] (SyncEvents::CopyData& data)
					{
						data.Orig_ = Util::FormatName (QFileInfo { data.Orig_ }.fileName ());
						data.CopySource_ = Util::FormatName (QFileInfo { data.CopySource_ }.fileName ());
					});
			return event;
		}
	}

	QString DevicesBrowserWidget::ToString (const SyncEvents::Event& event)
	{
		using namespace SyncEvents;
		const auto skip = u"🛡️ "_qs;
		const auto start = u"⏳ "_qs;
		const auto finish = u"✅ "_qs;
		const auto error = u"❌ "_qs;
		return Util::Visit (PrepareStrings (event),
				[&] (const XcodingSkipped& e) { return skip + tr ("skipping transcoding %n lossy file(s)…", nullptr, e.Count_); },
				[&] (const XcodingStarted& e) { return start + tr ("transcoding %1…").arg (e.Orig_); },
				[&] (const XcodingFinished& e) { return finish + tr ("transcoded %1").arg (e.Orig_); },
				[&] (const XcodingFailed& e) { return error + tr ("failed to transcode %1 → %2: %3").arg (e.Orig_, e.Target_, e.Message_); },
				[&] (const CopyStarted& e) { return start + tr ("copying %1…").arg (e.Orig_); },
				[&] (const CopyFinished& e) { return finish + tr ("copied %2").arg (e.Orig_); },
				[&] (const CopyFailed& e) { return error + tr ("failed to copy %1 (from %2): %3").arg (e.Orig_, e.CopySource_, e.Message_); });
	}

	void DevicesBrowserWidget::HandleSyncEvent (const SyncEvents::Event& event)
	{
		const auto& timestamp = QTime::currentTime ().toString ("[HH:mm:ss.zzz] "_qs);
		const auto& text = timestamp + ToString (event);
		Ui_.UploadLog_->append (text);
	}
}
}

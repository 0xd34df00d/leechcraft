/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrenttabwidget.h"
#include <chrono>
#include <QAction>
#include <QUrl>
#include <QTimer>
#include <libtorrent/lazy_entry.hpp>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "peersmodel.h"
#include "piecesmodel.h"
#include "addwebseeddialog.h"
#include "sessionsettingsmanager.h"
#include "sessionstats.h"
#include "ltutils.h"

namespace LC::BitTorrent
{
	TorrentTabWidget::TorrentTabWidget (QWidget *parent)
	: QTabWidget (parent)
	{
		Ui_.setupUi (this);
		new Util::TagsCompleter (Ui_.TorrentTags_);
		auto header = Ui_.PerTrackerStats_->header ();
		const auto& fm = fontMetrics ();
		header->resizeSection (0, fm.horizontalAdvance ("www.domain.name.org"));
		header->resizeSection (1, fm.horizontalAdvance ("1234.5678 bytes/s"));
		header->resizeSection (2, fm.horizontalAdvance ("1234.5678 bytes/s"));

		Ui_.TorrentTags_->AddSelector ();

		header = Ui_.WebSeedsView_->header ();
		header->resizeSection (0, fm.horizontalAdvance ("average.domain.name.of.a.tracker"));
		header->resizeSection (1, fm.horizontalAdvance ("  BEP 99  "));

		connect (Core::Instance (),
				&Core::torrentsStatusesUpdated,
				this,
				&TorrentTabWidget::UpdateTorrentStats,
				Qt::QueuedConnection);
		connect (this,
				&QTabWidget::currentChanged,
				this,
				&TorrentTabWidget::UpdateTorrentStats);

		const auto addWebSeedAct = new QAction (tr ("Add web seed..."), Ui_.WebSeedsView_);
		connect (addWebSeedAct,
				&QAction::triggered,
				this,
				&TorrentTabWidget::AddWebSeed);
		Ui_.WebSeedsView_->addAction (addWebSeedAct);

		RemoveWebSeedAction_ = new QAction (tr ("Remove web seed"), Ui_.WebSeedsView_);
		RemoveWebSeedAction_->setProperty ("ActionIcon", "list-remove-user");
		RemoveWebSeedAction_->setEnabled (false);
		connect (RemoveWebSeedAction_,
				&QAction::triggered,
				this,
				&TorrentTabWidget::RemoveWebSeed);
		Ui_.WebSeedsView_->addAction (RemoveWebSeedAction_);

		connect (Ui_.LabelComment_,
				&QLabel::linkActivated,
				[] (const QString& link)
				{
					const auto& e = Util::MakeEntity (QUrl::fromEncoded (link.toUtf8 ()),
							{},
							TaskParameter::FromUserInitiated | TaskParameter::OnlyHandle);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});

		XmlSettingsManager::Instance ()->RegisterObject ({
					"ActiveSessionStats",
					"ActiveAdvancedSessionStats",
					"ActiveTrackerStats",
					"ActiveCacheStats",
					"ActiveTorrentStatus",
					"ActiveTorrentAdvancedStatus",
					"ActiveTorrentInfo",
					"ActiveTorrentPeers"
				},
				this, [this] { SetTabWidgetSettings (); });
	}

	void TorrentTabWidget::SetDependencies (const Dependencies& deps)
	{
		SSM_ = deps.SSM_;
		connect (Ui_.OverallDownloadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				SSM_,
				&SessionSettingsManager::SetOverallDownloadRate);
		connect (Ui_.OverallUploadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				SSM_,
				&SessionSettingsManager::SetOverallUploadRate);
		connect (Ui_.DownloadingTorrents_,
				qOverload<int> (&QSpinBox::valueChanged),
				SSM_,
				&SessionSettingsManager::SetMaxDownloadingTorrents);
		connect (Ui_.UploadingTorrents_,
				qOverload<int> (&QSpinBox::valueChanged),
				SSM_,
				&SessionSettingsManager::SetMaxUploadingTorrents);

		Holder_ = &deps.Holder_;
		Ui_.PagePeers_->SetSessionHolder (deps.Holder_);

		connect (Ui_.TorrentDownloadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				[this] (int val)
				{
					ForEachSelected ([this, val] (int idx) { SetDownloadLimit ((*Holder_) [idx], val); });
				});
		connect (Ui_.TorrentUploadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				[this] (int val)
				{
					ForEachSelected ([this, val] (int idx) { SetUploadLimit ((*Holder_) [idx], val); });
				});
		connect (Ui_.TorrentManaged_,
				&QCheckBox::stateChanged,
				[this] (int state)
				{
					const auto enable = state == Qt::Checked;
					ForEachSelected ([enable] (int idx) { Core::Instance ()->SetTorrentManaged (enable, idx); });
				});
		connect (Ui_.TorrentSequentialDownload_,
				&QCheckBox::stateChanged,
				[this] (int state)
				{
					const auto enable = state == Qt::Checked;
					ForEachSelected ([enable] (int idx) { Core::Instance ()->SetTorrentSequentialDownload (enable, idx); });
				});
		connect (Ui_.TorrentSuperSeeding_,
				&QCheckBox::stateChanged,
				[this] (int state)
				{
					const auto enable = state == Qt::Checked;
					ForEachSelected ([enable] (int idx) { Core::Instance ()->SetTorrentSuperSeeding (enable, idx); });
				});
		connect (Ui_.TorrentTags_,
				&QLineEdit::editingFinished,
				[this]
				{
					const auto& tags = GetProxyHolder ()->GetTagsManager ()->Split (Ui_.TorrentTags_->text ());
					ForEachSelected ([&tags] (int idx) { Core::Instance ()->UpdateTags (tags, idx); });
				});

		const auto timer = new QTimer { this };
		timer->setTimerType (Qt::VeryCoarseTimer);
		timer->callOnTimeout ([this]
				{
					Ui_.PagePeers_->Update ();
					if (PiecesModel_)
						PiecesModel_->Update ();
				});
		timer->start (2000);
	}

	void TorrentTabWidget::SetChangeTrackersAction (QAction *changeTrackers)
	{
		Ui_.TrackersButton_->setDefaultAction (changeTrackers);
	}

	void TorrentTabWidget::SetCurrentIndex (int index)
	{
		if (Index_ == index)
			return;

		Index_ = index;
		InvalidateSelection ();

		Ui_.FilesWidget_->SetCurrentIndex (Index_);

		auto newPiecesModel = std::make_unique<PiecesModel> (*Holder_, Index_);
		Ui_.PiecesView_->setModel (newPiecesModel.get ());
		PiecesModel_ = std::move (newPiecesModel);

		Ui_.PagePeers_->SetSelectedTorrent (Index_);

		auto newWebSeedsModel = MakeWebSeedsModel ((*Holder_) [Index_]);
		Ui_.WebSeedsView_->setModel (newWebSeedsModel.get ());
		connect (Ui_.WebSeedsView_->selectionModel (),
				&QItemSelectionModel::currentChanged,
				[this] (const QModelIndex& idx) { RemoveWebSeedAction_->setEnabled (idx.isValid ()); });
		WebSeedsModel_ = std::move (newWebSeedsModel);
	}

	void TorrentTabWidget::SetSelectedIndices (const QList<int>& indices)
	{
		SelectedIndices_ = indices;
	}

	void TorrentTabWidget::InvalidateSelection ()
	{
		Ui_.TorrentTags_->setText (Core::Instance ()->GetProxy ()->GetTagsManager ()->
				Join (Core::Instance ()->GetTagsForIndex (Index_)));
		UpdateTorrentStats ();
	}

	void TorrentTabWidget::UpdateTorrentStats ()
	{
		UpdateDashboard ();
		UpdateOverallStats ();
		UpdateTorrentControl ();
	}

	template<typename F>
	void TorrentTabWidget::ForEachSelected (F&& f) const
	{
		for (const auto& index : SelectedIndices_)
			f (index);
	}

	void TorrentTabWidget::UpdateOverallStats ()
	{
		const auto& stats = Core::Instance ()->GetSessionStats ();

		Ui_.LabelTotalDownloadRate_->setText (Util::MakePrettySize (stats.Rate_.Down_) + tr ("/s"));
		Ui_.LabelTotalUploadRate_->setText (Util::MakePrettySize (stats.Rate_.Up_) + tr ("/s"));

		auto rel = [] (auto fmt, auto num, auto denom, QLabel *down, QLabel *up)
		{
			down->setText (fmt (num.Down_, denom.Down_));
			up->setText (fmt (num.Up_, denom.Up_));
		};

		auto percent = [] (auto t1, auto t2)
		{
			if (t2)
				return " (" + QString::number (t1 * 100.0 / t2, 'f', 1) + "%)";
			else
				return QString {};
		};

		auto speed = [percent] (auto t1, auto t2)
		{
			return Util::MakePrettySize (t1) + QObject::tr ("/s") + percent (t1, t2);
		};
		rel (speed, stats.IPOverheadRate_, stats.Rate_, Ui_.LabelOverheadDownloadRate_, Ui_.LabelOverheadUploadRate_);
		rel (speed, stats.DHTRate_, stats.Rate_, Ui_.LabelDHTDownloadRate_, Ui_.LabelDHTUploadRate_);
		rel (speed, stats.TrackerRate_, stats.Rate_, Ui_.LabelTrackerDownloadRate_, Ui_.LabelTrackerUploadRate_);

		Ui_.LabelTotalDownloaded_->setText (Util::MakePrettySize (stats.Total_.Down_));
		Ui_.LabelTotalUploaded_->setText (Util::MakePrettySize (stats.Total_.Up_));

		auto simple = [percent] (auto t1, auto t2)
		{
			return Util::MakePrettySize (t1) + percent (t1, t2);
		};
		rel (simple, stats.IPOverheadTotal_, stats.Total_, Ui_.LabelOverheadDownloaded_, Ui_.LabelOverheadUploaded_);
		rel (simple, stats.DHTTotal_, stats.Total_, Ui_.LabelDHTDownloaded_, Ui_.LabelDHTUploaded_);
		rel (simple, stats.TrackerTotal_, stats.Total_, Ui_.LabelTrackerDownloaded_, Ui_.LabelTrackerUploaded_);

		Ui_.LabelTotalPeers_->setText (QString::number (stats.NumPeers_));
		Ui_.LabelTotalDHTNodes_->setText ("(" +
				QString::number (stats.DHTGlobalNodes_) +
				") " +
				QString::number (stats.DHTNodes_));
		Ui_.LabelDHTTorrents_->setText (QString::number (stats.DHTTorrents_));
		Ui_.LabelListenPort_->setText (QString::number (Core::Instance ()->GetListenPort ()));
		if (stats.PayloadTotal_.Down_)
			Ui_.LabelSessionRating_->setText (QString::number (stats.PayloadTotal_.Up_ /
					static_cast<double> (stats.PayloadTotal_.Down_), 'g', 4));
		else
			Ui_.LabelSessionRating_->setText (QString::fromUtf8 ("\u221E"));
		Ui_.LabelTotalFailedData_->setText (Util::MakePrettySize (stats.TotalFailedBytes_));
		Ui_.LabelTotalRedundantData_->setText (Util::MakePrettySize (stats.TotalRedundantBytes_));
		Ui_.LabelExternalAddress_->setText (Core::Instance ()->GetExternalAddress ());

		Ui_.BlocksWritten_->setText (QString::number (stats.BlocksWritten_));
		Ui_.Writes_->setText (QString::number (stats.Writes_));
		Ui_.WriteHitRatio_->setText (QString::number (static_cast<double> (stats.BlocksWritten_ - stats.Writes_) / stats.BlocksWritten_));
		Ui_.CacheSize_->setText (QString::number (stats.CacheSize_));
		Ui_.TotalBlocksRead_->setText (QString::number (stats.BlocksRead_));
		Ui_.CachedBlockReads_->setText (QString::number (stats.BlocksReadHit_));
		Ui_.ReadHitRatio_->setText (QString::number (static_cast<double> (stats.BlocksReadHit_) / stats.BlocksRead_));
		Ui_.ReadCacheSize_->setText (QString::number (stats.ReadCacheSize_));

		Core::pertrackerstats_t ptstats;
		Core::Instance ()->GetPerTracker (ptstats);
		Ui_.PerTrackerStats_->clear ();

		for (auto i = ptstats.begin (), end = ptstats.end (); i != end; ++i)
		{
			QStringList strings;
			strings	<< i.key ()
				<< Util::MakePrettySize (i->DownloadRate_) + tr ("/s")
				<< Util::MakePrettySize (i->UploadRate_) + tr ("/s");

			new QTreeWidgetItem (Ui_.PerTrackerStats_, strings);
		}
	}

	void TorrentTabWidget::UpdateDashboard ()
	{
		Ui_.OverallDownloadRateController_->setValue (SSM_->GetOverallDownloadRate ());
		Ui_.OverallUploadRateController_->setValue (SSM_->GetOverallUploadRate ());
		Ui_.DownloadingTorrents_->setValue (SSM_->GetMaxDownloadingTorrents ());
		Ui_.UploadingTorrents_->setValue (SSM_->GetMaxUploadingTorrents ());
	}

	void TorrentTabWidget::UpdateTorrentControl ()
	{
		Ui_.TorrentDownloadRateController_->setValue (GetDownloadLimit ((*Holder_) [Index_]));
		Ui_.TorrentUploadRateController_->setValue (GetUploadLimit ((*Holder_) [Index_]));
		Ui_.TorrentManaged_->setCheckState (Core::Instance ()->
				IsTorrentManaged (Index_) ? Qt::Checked : Qt::Unchecked);
		Ui_.TorrentSequentialDownload_->setCheckState (Core::Instance ()->
				IsTorrentSequentialDownload (Index_) ? Qt::Checked : Qt::Unchecked);
		Ui_.TorrentSuperSeeding_->setCheckState (Core::Instance ()->
				IsTorrentSuperSeeding (Index_) ? Qt::Checked : Qt::Unchecked);

		std::unique_ptr<TorrentInfo> i;
		try
		{
			i = Core::Instance ()->GetTorrentStats (Index_);
		}
		catch (...)
		{
			Ui_.TorrentControlTab_->setEnabled (false);
			return;
		}

		if (!i->Info_)
			i->Info_.reset (new libtorrent::torrent_info { libtorrent::bdecode_node {} });

		Ui_.TorrentControlTab_->setEnabled (true);
		Ui_.LabelState_->setText (i->State_);
		Ui_.LabelDownloadRate_->setText (Util::MakePrettySize (i->Status_.download_rate) + tr ("/s"));
		Ui_.LabelUploadRate_->setText (Util::MakePrettySize (i->Status_.upload_rate) + tr ("/s"));

		const auto nextAnnounceSecs = libtorrent::duration_cast<libtorrent::seconds>(i->Status_.next_announce).count ();
		Ui_.LabelNextAnnounce_->setText (Util::MakeTimeFromLong (nextAnnounceSecs));

		Ui_.LabelProgress_->setText (QString::number (i->Status_.progress * 100, 'f', 2) + "%");
		Ui_.LabelDownloaded_->setText (Util::MakePrettySize (i->Status_.total_download));
		Ui_.LabelUploaded_->setText (Util::MakePrettySize (i->Status_.total_upload));
		Ui_.LabelWantedDownloaded_->setText (Util::MakePrettySize (i->Status_.total_wanted_done));
		Ui_.LabelDownloadedTotal_->setText (Util::MakePrettySize (i->Status_.all_time_download));
		Ui_.LabelUploadedTotal_->setText (Util::MakePrettySize (i->Status_.all_time_upload));
		if (i->Status_.all_time_download)
			Ui_.LabelTorrentOverallRating_->setText (QString::number (i->Status_.all_time_upload /
							static_cast<double> (i->Status_.all_time_download), 'g', 4));
		else
			Ui_.LabelTorrentOverallRating_->setText (QString::fromUtf8 ("\u221E"));
		Ui_.LabelSeedRank_->setText (QString::number (i->Status_.seed_rank));
		Ui_.LabelActiveTime_->setText (Util::MakeTimeFromLong (i->Status_.active_duration.count ()));
		Ui_.LabelSeedingTime_->setText (Util::MakeTimeFromLong (i->Status_.seeding_duration.count ()));
		Ui_.LabelTotalSize_->setText (Util::MakePrettySize (i->Info_->total_size ()));
		Ui_.LabelWantedSize_->setText (Util::MakePrettySize (i->Status_.total_wanted));
		if (i->Status_.total_payload_download)
			Ui_.LabelTorrentRating_->setText (QString::number (i->Status_.total_payload_upload /
							static_cast<double> (i->Status_.total_payload_download), 'g', 4));
		else
			Ui_.LabelTorrentRating_->setText (QString::fromUtf8 ("\u221E"));
		Ui_.PiecesWidget_->setPieceMap (i->Status_.pieces);
		Ui_.LabelTracker_->setText (QString::fromStdString (i->Status_.current_tracker));
		Ui_.LabelDestination_->setText (QString ("<a href='%1'>%1</a>")
					.arg (i->Destination_));
		Ui_.LabelName_->setText (QString::fromStdString (i->Status_.name));
		Ui_.LabelCreator_->setText (QString::fromStdString (i->Info_->creator ()));

		const auto& commentString = QString::fromStdString (i->Info_->comment ());
		if (QUrl::fromEncoded (commentString.toUtf8 ()).isValid ())
			Ui_.LabelComment_->setText (QString ("<a href='%1'>%1</a>")
					.arg (commentString));
		else
			Ui_.LabelComment_->setText (commentString);

		Ui_.LabelPrivate_->setText (i->Info_->priv () ?
					tr ("Yes") :
					tr ("No"));
		Ui_.LabelDHTNodesCount_->setText (QString::number (i->Info_->nodes ().size ()));
		Ui_.LabelFailed_->setText (Util::MakePrettySize (i->Status_.total_failed_bytes));
		Ui_.LabelConnectedPeers_->setText (QString::number (i->Status_.num_peers));
		Ui_.LabelConnectedSeeds_->setText (QString::number (i->Status_.num_seeds));
		Ui_.LabelTotalPieces_->setText (QString::number (i->Info_->num_pieces ()));
		Ui_.LabelDownloadedPieces_->setText (QString::number (i->Status_.num_pieces));
		Ui_.LabelPieceSize_->setText (Util::MakePrettySize (i->Info_->piece_length ()));
		Ui_.LabelBlockSize_->setText (Util::MakePrettySize (i->Status_.block_size));
		Ui_.LabelDistributedCopies_->setText (i->Status_.distributed_copies == -1 ?
					tr ("Not tracking") :
					QString::number (i->Status_.distributed_copies));
		Ui_.LabelRedundantData_->setText (Util::MakePrettySize (i->Status_.total_redundant_bytes));
		Ui_.LabelPeersInList_->setText (QString::number (i->Status_.list_peers));
		Ui_.LabelSeedsInList_->setText (QString::number (i->Status_.list_seeds));
		Ui_.LabelPeersInSwarm_->setText ((i->Status_.num_incomplete == -1 ?
					tr ("Unknown") :
					QString::number (i->Status_.num_incomplete)));
		Ui_.LabelSeedsInSwarm_->setText ((i->Status_.num_complete == -1 ?
					tr ("Unknown") :
					QString::number (i->Status_.num_complete)));
		Ui_.LabelConnectCandidates_->setText (QString::number (i->Status_.connect_candidates));
		Ui_.LabelUpBandwidthQueue_->setText (QString::number (i->Status_.up_bandwidth_queue));
		Ui_.LabelDownBandwidthQueue_->setText (QString::number (i->Status_.down_bandwidth_queue));
	}

	void TorrentTabWidget::AddWebSeed ()
	{
		AddWebSeedDialog ws;
		if (ws.exec () != QDialog::Accepted)
			return;

		const auto& url = ws.GetURL ();
		if (url.isEmpty () || !QUrl { ws.GetURL () }.isValid ())
			return;

		auto& handle = (*Holder_) [Index_];
		switch (ws.GetType ())
		{
		case WebSeedType::Bep17:
			handle.add_http_seed (url.toStdString ());
			break;
		case WebSeedType::Bep19:
			handle.add_url_seed (url.toStdString ());
			break;
		}
	}

	void TorrentTabWidget::RemoveWebSeed ()
	{
		auto index = Ui_.WebSeedsView_->currentIndex ();
		auto url = index.sibling (index.row (), 0).data ().toString ().toStdString ();
		auto type = index.sibling (index.row (), 1).data ().toString () == "BEP 19" ?
				WebSeedType::Bep19 :
				WebSeedType::Bep17;

		auto& handle = (*Holder_) [Index_];
		switch (type)
		{
		case WebSeedType::Bep17:
			handle.remove_http_seed (url);
			break;
		case WebSeedType::Bep19:
			handle.remove_url_seed (url);
			break;
		}
	}

	void TorrentTabWidget::SetTabWidgetSettings ()
	{
		auto xsm = XmlSettingsManager::Instance ();
		Ui_.BoxSessionStats_->setVisible (xsm->property ("ActiveSessionStats").toBool ());
		Ui_.BoxAdvancedSessionStats_->setVisible (xsm->property ("ActiveAdvancedSessionStats").toBool ());
		Ui_.BoxPerTrackerStats_->setVisible (xsm->property ("ActiveTrackerStats").toBool ());
		Ui_.BoxCacheStats_->setVisible (xsm->property ("ActiveCacheStats").toBool ());
		Ui_.BoxTorrentStatus_->setVisible (xsm->property ("ActiveTorrentStatus").toBool ());
		Ui_.BoxTorrentAdvancedStatus_->setVisible (xsm->property ("ActiveTorrentAdvancedStatus").toBool ());
		Ui_.BoxTorrentInfo_->setVisible (xsm->property ("ActiveTorrentInfo").toBool ());
		Ui_.BoxTorrentPeers_->setVisible (xsm->property ("ActiveTorrentPeers").toBool ());
	}
}

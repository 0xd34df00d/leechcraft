/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tabwidget.h"
#include <util/util.h>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "torrentfilesmodel.h"
#include "xmlsettingsmanager.h"
#include "addwebseeddialog.h"
#include "sessionsettingsmanager.h"
#include "sessionstats.h"
#include "ltutils.h"

namespace LC::BitTorrent
{
	TabWidget::TabWidget (QAbstractItemModel& model, libtorrent::session& session, SessionSettingsManager& ssm, QWidget *parent)
	: QWidget { parent }
	, Session_ { session }
	, SSM_ { ssm }
	, Model_ { model }
	{
		Ui_.setupUi (this);

		new Util::TagsCompleter { Ui_.TorrentTags_ };
		Ui_.TorrentTags_->AddSelector ();

		connect (&model,
				&QAbstractItemModel::dataChanged,
				this,
				[this] (const QModelIndex& from, const QModelIndex& to)
				{
					if (from.row () <= Torrent_.row () && Torrent_.row () <= to.row ())
						UpdateTorrentStats ();
				});

		UpdateDashboard ();

		connect (Ui_.OverallDownloadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				&SSM_,
				&SessionSettingsManager::SetOverallDownloadRate);
		connect (Ui_.OverallUploadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				&SSM_,
				&SessionSettingsManager::SetOverallUploadRate);
		connect (Ui_.TorrentDownloadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				[this] (int val) { SetDownloadLimit (GetTorrentHandle (Torrent_), val); });
		connect (Ui_.TorrentUploadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				[this] (int val) { SetUploadLimit (GetTorrentHandle (Torrent_), val); });
		connect (Ui_.TorrentManaged_,
				&QCheckBox::clicked,
				[this] (bool managed) { Model_.setData (Torrent_, managed, Roles::IsManaged); });
		connect (Ui_.TorrentSequentialDownload_,
				&QCheckBox::clicked,
				[this] (bool sequential) { Model_.setData (Torrent_, sequential, Roles::IsSequentialDownloading); });
		connect (Ui_.DownloadingTorrents_,
				qOverload<int> (&QSpinBox::valueChanged),
				&SSM_,
				&SessionSettingsManager::SetMaxDownloadingTorrents);
		connect (Ui_.UploadingTorrents_,
				qOverload<int> (&QSpinBox::valueChanged),
				&SSM_,
				&SessionSettingsManager::SetMaxUploadingTorrents);
		connect (Ui_.TorrentTags_,
				&QLineEdit::editingFinished,
				[this]
				{
					const auto& tags = GetProxyHolder ()->GetTagsManager ()->Split (Ui_.TorrentTags_->text ());
					Model_.setData (Torrent_, tags, Roles::TorrentTags);
				});
	}

	QModelIndex TabWidget::GetCurrentTorrent () const
	{
		return Torrent_;
	}

	void TabWidget::SetCurrentTorrent (const QModelIndex& torrent)
	{
		Q_ASSERT (torrent.model () == &Model_);

		Torrent_ = torrent;

		if (!Torrent_.isValid ())
			return;

		const auto& tags = Torrent_.data (Roles::TorrentTags).toStringList ();
		Ui_.TorrentTags_->setText (GetProxyHolder ()->GetTagsManager ()->Join (tags));
		UpdateTorrentStats ();
	}

	void TabWidget::UpdateTorrentStats ()
	{
		if (!Torrent_.isValid ())
			return;

		UpdateTorrentControl ();
		UpdateDashboard ();
		UpdateOverallStats ();
	}

	void TabWidget::UpdateOverallStats ()
	{
		const auto& stats = Core::Instance ()->GetSessionStats ();
		Ui_.LabelTotalDownloadRate_->setText (Util::MakePrettySize (stats.Rate_.Down_) + tr ("/s"));
		Ui_.LabelTotalUploadRate_->setText (Util::MakePrettySize (stats.Rate_.Up_) + tr ("/s"));
	}

	void TabWidget::UpdateDashboard ()
	{
		Ui_.OverallDownloadRateController_->setValue (SSM_.GetOverallDownloadRate ());
		Ui_.OverallUploadRateController_->setValue (SSM_.GetOverallUploadRate ());
		Ui_.DownloadingTorrents_->setValue (SSM_.GetMaxDownloadingTorrents ());
		Ui_.UploadingTorrents_->setValue (SSM_.GetMaxUploadingTorrents ());
	}

	void TabWidget::UpdateTorrentControl ()
	{
		const auto& handle = GetTorrentHandle (Torrent_);
		Ui_.TorrentDownloadRateController_->setValue (GetDownloadLimit (handle));
		Ui_.TorrentUploadRateController_->setValue (GetUploadLimit (handle));
		Ui_.TorrentManaged_->setCheckState (Torrent_.data (Roles::IsManaged).toBool () ?
					Qt::Checked :
					Qt::Unchecked);

		Ui_.TorrentSequentialDownload_->setCheckState (Torrent_.data (Roles::IsSequentialDownloading).toBool () ?
				Qt::Checked :
				Qt::Unchecked);

		const auto& info = Torrent_.data (Roles::TorrentStats).value<TorrentInfo> ();

		Ui_.TorrentSettingsBox_->setEnabled (true);
		Ui_.LabelState_->setText (info.State_);
		Ui_.LabelDownloadRate_->setText (Util::MakePrettySize (info.Status_.download_rate) + tr ("/s"));
		Ui_.LabelUploadRate_->setText (Util::MakePrettySize (info.Status_.upload_rate) + tr ("/s"));
		Ui_.LabelProgress_->setText (QString::number (info.Status_.progress * 100, 'f', 2) + "%");
		Ui_.LabelWantedDownloaded_->setText (Util::MakePrettySize (info.Status_.total_wanted_done));
		Ui_.LabelWantedSize_->setText (Util::MakePrettySize (info.Status_.total_wanted));
		Ui_.LabelTotalUploaded_->setText (Util::MakePrettySize (info.Status_.all_time_upload));
		Ui_.PiecesWidget_->SetPieceMap (info.Status_.pieces);
		Ui_.LabelName_->setText (QString::fromStdString (info.Status_.name));
	}
}

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
#include "peersmodel.h"
#include "addwebseeddialog.h"
#include "sessionsettingsmanager.h"
#include "sessionstats.h"
#include "ltutils.h"

namespace LC::BitTorrent
{
	TabWidget::TabWidget (SessionHolder& holder, SessionSettingsManager& ssm, QWidget *parent)
	: QWidget { parent }
	, Holder_ { holder }
	, SSM_ { ssm }
	{
		Ui_.setupUi (this);

		new Util::TagsCompleter { Ui_.TorrentTags_ };
		Ui_.TorrentTags_->AddSelector ();

		connect (Core::Instance (),
				&QAbstractItemModel::dataChanged,
				this,
				[this] (const QModelIndex& from, const QModelIndex& to)
				{
					if (from.data (Roles::HandleIndex).toInt () <= Torrent_ &&
							Torrent_ <= to.data (Roles::HandleIndex).toInt ())
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
				[this] (int val) { SetDownloadLimit (Holder_ [Torrent_], val); });
		connect (Ui_.TorrentUploadRateController_,
				qOverload<int> (&QSpinBox::valueChanged),
				[this] (int val) { SetUploadLimit (Holder_ [Torrent_], val); });
		connect (Ui_.TorrentManaged_,
				&QCheckBox::clicked,
				[this] (bool managed) { Core::Instance ()->SetTorrentManaged (managed, Torrent_); });
		connect (Ui_.TorrentSequentialDownload_,
				&QCheckBox::clicked,
				[this] (bool managed) { Core::Instance ()->SetTorrentSequentialDownload (managed, Torrent_); });
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
					const auto& split = GetProxyHolder ()->GetTagsManager ()->Split (Ui_.TorrentTags_->text ());
					Core::Instance ()->UpdateTags (split, Torrent_);
				});
	}

	int TabWidget::GetCurrentTorrent () const
	{
		return Torrent_;
	}

	void TabWidget::SetCurrentTorrent (int torrent)
	{
		Torrent_ = torrent;

		if (Torrent_ == -1)
			return;

		const auto& tags = Core::Instance ()->GetTagsForIndex (Torrent_);
		Ui_.TorrentTags_->setText (GetProxyHolder ()->GetTagsManager ()->Join (tags));
		UpdateTorrentStats ();
	}

	void TabWidget::UpdateTorrentStats ()
	{
		if (Torrent_ == -1)
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
		Ui_.OverallDownloadRateController_->setValue (SSM_->GetOverallDownloadRate ());
		Ui_.OverallUploadRateController_->setValue (SSM_->GetOverallUploadRate ());
		Ui_.DownloadingTorrents_->setValue (SSM_->GetMaxDownloadingTorrents ());
		Ui_.UploadingTorrents_->setValue (SSM_->GetMaxUploadingTorrents ());
	}

	void TabWidget::UpdateTorrentControl ()
	{
		Ui_.TorrentDownloadRateController_->setValue (GetDownloadLimit (Holder_ [Torrent_]));
		Ui_.TorrentUploadRateController_->setValue (GetUploadLimit (Holder_ [Torrent_]));
		Ui_.TorrentManaged_->setCheckState (Core::Instance ()->IsTorrentManaged (Torrent_) ?
					Qt::Checked :
					Qt::Unchecked);

		Ui_.TorrentSequentialDownload_->setCheckState (Core::Instance ()->
				IsTorrentSequentialDownload (Torrent_) ? Qt::Checked : Qt::Unchecked);

		std::unique_ptr<TorrentInfo> i;
		try
		{
			i = Core::Instance ()->GetTorrentStats (Torrent_);
		}
		catch (...)
		{
			Ui_.TorrentSettingsBox_->setEnabled (false);
			return;
		}

		Ui_.TorrentSettingsBox_->setEnabled (true);
		Ui_.LabelState_->setText (i->State_);
		Ui_.LabelDownloadRate_->setText (Util::MakePrettySize (i->Status_.download_rate) + tr ("/s"));
		Ui_.LabelUploadRate_->setText (Util::MakePrettySize (i->Status_.upload_rate) + tr ("/s"));
		Ui_.LabelProgress_->setText (QString::number (i->Status_.progress * 100, 'f', 2) + "%");
		Ui_.LabelWantedDownloaded_->setText (Util::MakePrettySize (i->Status_.total_wanted_done));
		Ui_.LabelWantedSize_->setText (Util::MakePrettySize (i->Status_.total_wanted));
		Ui_.LabelTotalUploaded_->setText (Util::MakePrettySize (i->Status_.all_time_upload));
		Ui_.PiecesWidget_->SetPieceMap (i->Status_.pieces);
		Ui_.LabelName_->setText (QString::fromStdString (i->Status_.name));
	}
}

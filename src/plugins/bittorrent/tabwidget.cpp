/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tabwidget.h"
#include <QSortFilterProxyModel>
#include <QUrl>
#include <util/util.h>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "torrentfilesmodel.h"
#include "xmlsettingsmanager.h"
#include "piecesmodel.h"
#include "peersmodel.h"
#include "addwebseeddialog.h"
#include "sessionsettingsmanager.h"
#include "sessionstats.h"
#include "ltutils.h"

namespace LC
{
namespace BitTorrent
{
	TabWidget::TabWidget (SessionHolder& holder, QWidget *parent)
	: QWidget { parent }
	, Holder_ { holder }
	{
		Ui_.setupUi (this);

		TagsChangeCompleter_ = new Util::TagsCompleter { Ui_.TorrentTags_ };
		Ui_.TorrentTags_->AddSelector ();

		connect (Core::Instance (),
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (updateTorrentStats (QModelIndex, QModelIndex)));

		UpdateDashboard ();
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

		TorrentSelectionChanged_ = true;
		const auto& tags = Core::Instance ()->GetTagsForIndex (Torrent_);
		Ui_.TorrentTags_->setText (GetProxyHolder ()->GetTagsManager ()->Join (tags));
		updateTorrentStats ();
	}

	void TabWidget::updateTorrentStats (const QModelIndex& from, const QModelIndex& to)
	{
		if (from.row () <= Torrent_ && Torrent_ <= to.row ())
			updateTorrentStats ();
	}

	void TabWidget::updateTorrentStats ()
	{
		if (Torrent_ == -1)
			return;

		UpdateTorrentControl ();
		UpdateDashboard ();
		UpdateOverallStats ();
		TorrentSelectionChanged_ = false;
	}

	void TabWidget::UpdateOverallStats ()
	{
		const auto& stats = Core::Instance ()->GetSessionStats ();
		Ui_.LabelTotalDownloadRate_->setText (Util::MakePrettySize (stats.Rate_.Down_) + tr ("/s"));
		Ui_.LabelTotalUploadRate_->setText (Util::MakePrettySize (stats.Rate_.Up_) + tr ("/s"));
	}

	void TabWidget::UpdateDashboard ()
	{
		const auto ssm = Core::Instance ()->GetSessionSettingsManager ();
		Ui_.OverallDownloadRateController_->setValue (ssm->GetOverallDownloadRate ());
		Ui_.OverallUploadRateController_->setValue (ssm->GetOverallUploadRate ());
		Ui_.DownloadingTorrents_->setValue (ssm->GetMaxDownloadingTorrents ());
		Ui_.UploadingTorrents_->setValue (ssm->GetMaxUploadingTorrents ());
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
		Ui_.PiecesWidget_->setPieceMap (i->Status_.pieces);
		Ui_.LabelName_->setText (QString::fromStdString (i->Status_.name));
	}

	void TabWidget::on_OverallDownloadRateController__valueChanged (int val)
	{
		Core::Instance ()->GetSessionSettingsManager ()->SetOverallDownloadRate (val);
	}

	void TabWidget::on_OverallUploadRateController__valueChanged (int val)
	{
		Core::Instance ()->GetSessionSettingsManager ()->SetOverallUploadRate (val);
	}

	void TabWidget::on_TorrentDownloadRateController__valueChanged (int val)
	{
		SetDownloadLimit (Holder_ [Torrent_], val);
	}

	void TabWidget::on_TorrentUploadRateController__valueChanged (int val)
	{
		SetUploadLimit (Holder_ [Torrent_], val);
	}

	void TabWidget::on_TorrentManaged__clicked (bool managed)
	{
		Core::Instance ()->SetTorrentManaged (managed, Torrent_);
	}

	void TabWidget::on_TorrentSequentialDownload__clicked (bool managed)
	{
		Core::Instance ()->SetTorrentSequentialDownload (managed, Torrent_);
	}

	void TabWidget::on_DownloadingTorrents__valueChanged (int newValue)
	{
		Core::Instance ()->GetSessionSettingsManager ()->SetMaxDownloadingTorrents (newValue);
	}

	void TabWidget::on_UploadingTorrents__valueChanged (int newValue)
	{
		Core::Instance ()->GetSessionSettingsManager ()->SetMaxUploadingTorrents (newValue);
	}

	void TabWidget::on_TorrentTags__editingFinished ()
	{
		const auto& split = GetProxyHolder ()->GetTagsManager ()->Split (Ui_.TorrentTags_->text ());
		Core::Instance ()->UpdateTags (split, Torrent_);
	}
}
}

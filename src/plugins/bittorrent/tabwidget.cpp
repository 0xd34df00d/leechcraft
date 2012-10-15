/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "tabwidget.h"
#include <QSortFilterProxyModel>
#include <QUrl>
#include <util/util.h>
#include <util/models/treeitem.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "filesviewdelegate.h"
#include "torrentfilesmodel.h"
#include "xmlsettingsmanager.h"
#include "peerstablinker.h"
#include "piecesmodel.h"
#include "peersmodel.h"
#include "addpeerdialog.h"
#include "addwebseeddialog.h"
#include "banpeersdialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			using namespace Util;

			TabWidget::TabWidget (QWidget *parent)
			: QWidget (parent)
			, TorrentSelectionChanged_ (false)
			{
				Ui_.setupUi (this);
				QFontMetrics fm = QApplication::fontMetrics ();

				TagsChangeCompleter_.reset (new TagsCompleter (Ui_.TorrentTags_));
				Ui_.TorrentTags_->AddSelector ();

				connect (Ui_.OverallDownloadRateController_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_OverallDownloadRateController__valueChanged (int)));
				connect (Ui_.OverallUploadRateController_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_OverallUploadRateController__valueChanged (int)));
				connect (Ui_.TorrentDownloadRateController_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_TorrentDownloadRateController__valueChanged (int)));
				connect (Ui_.TorrentUploadRateController_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_TorrentUploadRateController__valueChanged (int)));
				connect (Ui_.TorrentManaged_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (on_TorrentManaged__stateChanged (int)));
				connect (Ui_.TorrentSequentialDownload_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (on_TorrentSequentialDownload__stateChanged (int)));
				connect (Ui_.DownloadingTorrents_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_DownloadingTorrents__valueChanged (int)));
				connect (Ui_.UploadingTorrents_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_UploadingTorrents__valueChanged (int)));
				connect (Ui_.TorrentTags_,
						SIGNAL (editingFinished ()),
						this,
						SLOT (on_TorrentTags__editingFinished ()));

				connect (Core::Instance (),
						SIGNAL (dataChanged (const QModelIndex&,
								const QModelIndex&)),
						this,
						SLOT (updateTorrentStats ()));
				connect (this,
						SIGNAL (currentChanged (int)),
						this,
						SLOT (updateTorrentStats ()));

				UpdateDashboard ();
			}

			void TabWidget::InvalidateSelection ()
			{
				TorrentSelectionChanged_ = true;
				Ui_.TorrentTags_->setText (Core::Instance ()->GetProxy ()->GetTagsManager ()->
						Join (Core::Instance ()->GetTagsForIndex ()));
				updateTorrentStats ();
			}

			void TabWidget::SetOverallDownloadRateController (int val)
			{
				Ui_.OverallDownloadRateController_->setValue (val);
				on_OverallDownloadRateController__valueChanged (val);
			}

			void TabWidget::SetOverallUploadRateController (int val)
			{
				Ui_.OverallUploadRateController_->setValue (val);
				on_OverallUploadRateController__valueChanged (val);
			}

			void TabWidget::updateTorrentStats ()
			{
				if (Core::Instance ()->GetCurrentTorrent () == -1)
					return;

				UpdateTorrentControl ();
				UpdateDashboard ();
				UpdateOverallStats ();
				TorrentSelectionChanged_ = false;
			}

			void TabWidget::UpdateOverallStats ()
			{
				const auto& stats = Core::Instance ()->GetOverallStats ();
				Ui_.LabelTotalDownloadRate_->
					setText (Util::MakePrettySize (stats.download_rate) + tr ("/s"));
				Ui_.LabelTotalUploadRate_->
					setText (Util::MakePrettySize (stats.upload_rate) + tr ("/s"));
			}

			void TabWidget::UpdateDashboard ()
			{
				Ui_.OverallDownloadRateController_->setValue (Core::Instance ()->GetOverallDownloadRate ());
				Ui_.OverallUploadRateController_->setValue (Core::Instance ()->GetOverallUploadRate ());
				Ui_.DownloadingTorrents_->setValue (Core::Instance ()->GetMaxDownloadingTorrents ());
				Ui_.UploadingTorrents_->setValue (Core::Instance ()->GetMaxUploadingTorrents ());
			}

			void TabWidget::UpdateTorrentControl ()
			{
				Ui_.TorrentDownloadRateController_->
						setValue (Core::Instance ()->GetTorrentDownloadRate (Core::Instance ()->GetCurrentTorrent ()));
				Ui_.TorrentUploadRateController_->setValue (Core::Instance ()->
						GetTorrentUploadRate (Core::Instance ()->GetCurrentTorrent ()));
				Ui_.TorrentManaged_->setCheckState (Core::Instance ()->
						IsTorrentManaged (Core::Instance ()->GetCurrentTorrent ()) ? Qt::Checked : Qt::Unchecked);
				Ui_.TorrentSequentialDownload_->setCheckState (Core::Instance ()->
						IsTorrentSequentialDownload (Core::Instance ()->GetCurrentTorrent ()) ? Qt::Checked : Qt::Unchecked);

				std::unique_ptr<TorrentInfo> i;
				try
				{
					i = Core::Instance ()->GetTorrentStats (Core::Instance ()->GetCurrentTorrent ());
				}
				catch (...)
				{
					Ui_.TorrentSettingsBox_->setEnabled (false);
					return;
				}

				Ui_.TorrentSettingsBox_->setEnabled (true);
				Ui_.LabelState_->setText (i->State_);
				Ui_.LabelDownloadRate_->
					setText (Util::MakePrettySize (i->Status_.download_rate) + tr ("/s"));
				Ui_.LabelUploadRate_->
					setText (Util::MakePrettySize (i->Status_.upload_rate) + tr ("/s"));
				Ui_.LabelProgress_->
					setText (QString::number (i->Status_.progress * 100, 'f', 2) + "%");
				Ui_.LabelWantedDownloaded_->
					setText (Util::MakePrettySize (i->Status_.total_wanted_done));
				Ui_.LabelWantedSize_->
					setText (Util::MakePrettySize (i->Status_.total_wanted));
				Ui_.LabelTotalUploaded_->
					setText (Util::MakePrettySize (i->Status_.all_time_upload));
				Ui_.PiecesWidget_->setPieceMap (i->Status_.pieces);
				Ui_.LabelName_->
					setText (QString::fromUtf8 (i->Info_->name ().c_str ()));
			}

			void TabWidget::on_OverallDownloadRateController__valueChanged (int val)
			{
				Core::Instance ()->SetOverallDownloadRate (val);
			}

			void TabWidget::on_OverallUploadRateController__valueChanged (int val)
			{
				Core::Instance ()->SetOverallUploadRate (val);
			}

			void TabWidget::on_TorrentDownloadRateController__valueChanged (int val)
			{
				Core::Instance ()->SetTorrentDownloadRate (val, Core::Instance ()->GetCurrentTorrent ());
			}

			void TabWidget::on_TorrentUploadRateController__valueChanged (int val)
			{
				Core::Instance ()->SetTorrentUploadRate (val, Core::Instance ()->GetCurrentTorrent ());
			}

			void TabWidget::on_TorrentManaged__stateChanged (int state)
			{
				Core::Instance ()->SetTorrentManaged (state == Qt::Checked, Core::Instance ()->GetCurrentTorrent ());
			}

			void TabWidget::on_TorrentSequentialDownload__stateChanged (int state)
			{
				Core::Instance ()->SetTorrentSequentialDownload (state == Qt::Checked, Core::Instance ()->GetCurrentTorrent ());
			}

			void TabWidget::on_DownloadingTorrents__valueChanged (int newValue)
			{
				Core::Instance ()->SetMaxDownloadingTorrents (newValue);
			}

			void TabWidget::on_UploadingTorrents__valueChanged (int newValue)
			{
				Core::Instance ()->SetMaxUploadingTorrents (newValue);
			}

			void TabWidget::on_TorrentTags__editingFinished ()
			{
				const auto& split = Core::Instance ()->GetProxy ()->
						GetTagsManager ()->Split (Ui_.TorrentTags_->text ());
				Core::Instance ()->UpdateTags (split, Core::Instance ()->GetCurrentTorrent ());
			}
		}
	};
};

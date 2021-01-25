/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2015  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrenttab.h"
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QToolBar>
#include <QMenu>
#include <libtorrent/session.hpp>
#include <libtorrent/announce_entry.hpp>
#include <util/tags/tagscompleter.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/gui/lineeditbuttonmanager.h>
#include <util/gui/util.h>
#include <util/sll/prelude.h>
#include <util/sll/views.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "addtorrent.h"
#include "addmultipletorrents.h"
#include "ipfilterdialog.h"
#include "newtorrentwizard.h"
#include "trackerschanger.h"
#include "movetorrentfiles.h"
#include "tabviewproxymodel.h"
#include "addmagnetdialog.h"
#include "xmlsettingsmanager.h"
#include "types.h"
#include "newtorrentparams.h"

namespace LC
{
namespace BitTorrent
{
	namespace
	{
		class TorrentsListDelegate : public QStyledItemDelegate
		{
		public:
			using QStyledItemDelegate::QStyledItemDelegate;

			void paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
			{
				if (index.column () != Core::ColumnProgress)
				{
					QStyledItemDelegate::paint (painter, option, index);
					return;
				}

				const auto progress = index.data (Core::SortRole).toDouble ();

				QStyleOptionProgressBar pbo;
				pbo.rect = option.rect;
				pbo.minimum = 0;
				pbo.maximum = 1000;
				pbo.progress = std::round (progress * 1000);
				pbo.state = option.state;
				pbo.text = Util::ElideProgressBarText (index.data ().toString (), option);
				pbo.textVisible = true;
				QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
			}
		};
	}

	TorrentTab::TorrentTab (const SessionHolder& holder, const TabClassInfo& tc, QObject *mt)
	: Holder_ { holder }
	, TC_ { tc }
	, ParentMT_ { mt }
	, Toolbar_ { new QToolBar { "BitTorrent" } }
	, ViewFilter_ { new TabViewProxyModel { this } }
	{
		Ui_.setupUi (this);
		Ui_.Tabs_->SetDependencies ({
				Core::Instance ()->GetSessionSettingsManager (),
				holder
			});

		ViewFilter_->setDynamicSortFilter (true);
		ViewFilter_->setSortRole (Core::Roles::SortRole);
		ViewFilter_->setSourceModel (Core::Instance ());

		Ui_.TorrentsView_->setItemDelegate (new TorrentsListDelegate (Ui_.TorrentsView_));

		Ui_.TorrentsView_->setModel (ViewFilter_);
		connect (Ui_.TorrentsView_->selectionModel (),
				&QItemSelectionModel::currentChanged,
				[this] (const QModelIndex& index)
				{
					Ui_.Tabs_->SetCurrentIndex (ViewFilter_->mapToSource (index).row ());
				});
		connect (Ui_.TorrentsView_->selectionModel (),
				&QItemSelectionModel::selectionChanged,
				[this] { Ui_.Tabs_->SetSelectedIndices (GetSelectedRows ()); });
		Ui_.TorrentsView_->sortByColumn (Core::ColumnID, Qt::SortOrder::AscendingOrder);

		QHeaderView *header = Ui_.TorrentsView_->header ();
		const auto& fm = fontMetrics ();
		header->resizeSection (Core::Columns::ColumnID,
				fm.horizontalAdvance ("999"));
		header->resizeSection (Core::Columns::ColumnName,
				fm.horizontalAdvance ("boardwalk.empire.s03e02.hdtv.720p.ac3.rus.eng.novafilm.tv.mkv") * 1.3);

		auto buttonMgr = new Util::LineEditButtonManager (Ui_.SearchLine_);
		new Util::TagsCompleter (Ui_.SearchLine_);
		Ui_.SearchLine_->AddSelector (buttonMgr);
		new Util::ClearLineEditAddon (Core::Instance ()->GetProxy (), Ui_.SearchLine_, buttonMgr);
		connect (Ui_.SearchLine_,
				SIGNAL (textChanged (QString)),
				ViewFilter_,
				SLOT (setFilterFixedString (QString)));

		connect (Ui_.TorrentStateFilter_,
				SIGNAL (currentIndexChanged (int)),
				ViewFilter_,
				SLOT (setStateFilterMode (int)));

		OpenTorrent_ = new QAction (tr ("Open torrent..."), Toolbar_);
		OpenTorrent_->setShortcut (Qt::Key_Insert);
		OpenTorrent_->setProperty ("ActionIcon", "document-open");
		connect (OpenTorrent_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenTorrentTriggered ()));

		AddMagnet_ = new QAction (tr ("Add magnet link..."), Toolbar_);
		AddMagnet_->setProperty ("ActionIcon", "document-open-remote");
		connect (AddMagnet_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddMagnetTriggered ()));

		CreateTorrent_ = new QAction (tr ("Create torrent..."), Toolbar_);
		CreateTorrent_->setProperty ("ActionIcon", "document-new");
		connect (CreateTorrent_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCreateTorrentTriggered ()));

		OpenMultipleTorrents_ = new QAction (tr ("Open multiple torrents..."), Toolbar_);
		OpenMultipleTorrents_->setProperty ("ActionIcon", "document-open-folder");
		connect (OpenMultipleTorrents_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenMultipleTorrentsTriggered ()));

		IPFilter_ = new QAction (tr ("IP filter..."), Toolbar_);
		IPFilter_->setProperty ("ActionIcon", "view-filter");
		connect (IPFilter_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleIPFilterTriggered ()));

		RemoveTorrent_ = new QAction (tr ("Remove"), Toolbar_);
		RemoveTorrent_->setShortcut (tr ("Del"));
		RemoveTorrent_->setProperty ("ActionIcon", "list-remove");
		connect (RemoveTorrent_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleRemoveTorrentTriggered ()));

		Resume_ = new QAction (tr ("Resume"), Toolbar_);
		Resume_->setShortcut (tr ("R"));
		Resume_->setProperty ("ActionIcon", "media-playback-start");
		connect (Resume_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleResumeTriggered ()));

		Stop_ = new QAction (tr ("Pause"), Toolbar_);
		Stop_->setShortcut (tr ("S"));
		Stop_->setProperty ("ActionIcon", "media-playback-pause");
		connect (Stop_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleStopTriggered ()));

		MoveUp_ = new QAction (tr ("Move up"), Toolbar_);
		MoveUp_->setShortcut (Qt::CTRL + Qt::Key_Up);
		MoveUp_->setProperty ("ActionIcon", "go-up");
		connect (MoveUp_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveUpTriggered ()));

		MoveDown_ = new QAction (tr ("Move down"), Toolbar_);
		MoveDown_->setShortcut (Qt::CTRL + Qt::Key_Down);
		MoveDown_->setProperty ("ActionIcon", "go-down");
		connect (MoveDown_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveDownTriggered ()));

		MoveToTop_ = new QAction (tr ("Move to top"), Toolbar_);
		MoveToTop_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
		MoveToTop_->setProperty ("ActionIcon", "go-top");
		connect (MoveToTop_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveToTopTriggered ()));

		MoveToBottom_ = new QAction (tr ("Move to bottom"), Toolbar_);
		MoveToBottom_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
		MoveToBottom_->setProperty ("ActionIcon", "go-bottom");
		connect (MoveToBottom_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveToBottomTriggered ()));

		ForceReannounce_ = new QAction (tr ("Reannounce"), Toolbar_);
		ForceReannounce_->setShortcut (tr ("F"));
		ForceReannounce_->setProperty ("ActionIcon", "network-wireless");
		connect (ForceReannounce_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleForceReannounceTriggered ()));

		ForceRecheck_ = new QAction (tr ("Recheck"), Toolbar_);
		ForceRecheck_->setProperty ("ActionIcon", "tools-check-spelling");
		connect (ForceRecheck_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleForceRecheckTriggered ()));

		MoveFiles_ = new QAction (tr ("Move files..."), Toolbar_);
		MoveFiles_->setShortcut (tr ("M"));
		MoveFiles_->setProperty ("ActionIcon", "transform-move");
		connect (MoveFiles_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMoveFilesTriggered ()));

		ChangeTrackers_ = new QAction (tr ("Change trackers..."), Toolbar_);
		ChangeTrackers_->setShortcut (tr ("C"));
		ChangeTrackers_->setProperty ("ActionIcon", "view-media-playlist");
		connect (ChangeTrackers_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleChangeTrackersTriggered ()));
		Ui_.Tabs_->SetChangeTrackersAction (ChangeTrackers_);

		MakeMagnetLink_ = new QAction (tr ("Make magnet link..."), Toolbar_);
		MakeMagnetLink_->setProperty ("ActionIcon", "insert-link");
		connect (MakeMagnetLink_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleMakeMagnetLinkTriggered ()));

		Toolbar_->addAction (OpenTorrent_);
		Toolbar_->addAction (AddMagnet_);
		Toolbar_->addAction (OpenMultipleTorrents_);
		Toolbar_->addAction (RemoveTorrent_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (CreateTorrent_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (Resume_);
		Toolbar_->addAction (Stop_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (MoveUp_);
		Toolbar_->addAction (MoveDown_);
		Toolbar_->addAction (MoveToTop_);
		Toolbar_->addAction (MoveToBottom_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (ForceReannounce_);
		Toolbar_->addAction (ForceRecheck_);
		Toolbar_->addAction (MoveFiles_);
		Toolbar_->addAction (ChangeTrackers_);
		Toolbar_->addAction (MakeMagnetLink_);

		setActionsEnabled ();
		connect (Ui_.TorrentsView_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (setActionsEnabled ()));
		/*
		Toolbar_->addSeparator ();
		DownSelectorAction_ = new SpeedSelectorAction ("Down", this);
		DownSelectorAction_->handleSpeedsChanged ();
		Toolbar_->addAction (DownSelectorAction_);
		UpSelectorAction_ = new SpeedSelectorAction ("Up", this);
		UpSelectorAction_->handleSpeedsChanged ();
		Toolbar_->addAction (UpSelectorAction_);

		connect (DownSelectorAction_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleFastSpeedComboboxes ()));
		connect (UpSelectorAction_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleFastSpeedComboboxes ()));
				*/
	}

	TabClassInfo TorrentTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* TorrentTab::ParentMultiTabs ()
	{
		return ParentMT_;
	}

	void TorrentTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* TorrentTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void TorrentTab::SetCurrentTorrent (int row)
	{
		const auto& srcIdx = Core::Instance ()->index (row, 0);
		Ui_.TorrentsView_->setCurrentIndex (ViewFilter_->mapFromSource (srcIdx));
	}

	int TorrentTab::GetCurrentTorrent () const
	{
		return ViewFilter_->mapToSource (Ui_.TorrentsView_->currentIndex ()).row ();
	}

	QList<int> TorrentTab::GetSelectedRows () const
	{
		return Util::Map (GetSelectedRowIndexes (), &QModelIndex::row);
	}

	QModelIndexList TorrentTab::GetSelectedRowIndexes () const
	{
		return Util::Map (Ui_.TorrentsView_->selectionModel ()->selectedRows (),
				[&] (const auto& idx) { return ViewFilter_->mapToSource (idx); });
	}

	void TorrentTab::setActionsEnabled ()
	{
		const auto& actions =
		{
			Resume_, Stop_, MakeMagnetLink_, RemoveTorrent_,
			MoveUp_, MoveDown_, MoveToTop_, MoveToBottom_,
			ForceReannounce_, ForceRecheck_, MoveFiles_, ChangeTrackers_
		};
		const bool enable = Ui_.TorrentsView_->currentIndex ().isValid ();

		for (auto action : actions)
			action->setEnabled (enable);
	}

	void TorrentTab::on_TorrentsView__customContextMenuRequested (const QPoint& point)
	{
		QMenu menu;
		menu.addActions ({ Resume_, Stop_, MakeMagnetLink_, RemoveTorrent_ });
		menu.addSeparator ();
		menu.addActions ({ MoveToTop_, MoveUp_, MoveDown_, MoveToBottom_ });
		menu.addSeparator ();
		menu.addActions ({ ForceReannounce_, ForceRecheck_, MoveFiles_, ChangeTrackers_ });
		menu.exec (Ui_.TorrentsView_->viewport ()->mapToGlobal (point));
	}

	void TorrentTab::handleOpenTorrentTriggered ()
	{
		const auto dia = new AddTorrent (this);
		connect (dia,
				&QDialog::accepted,
				this,
				[this, dia]
				{
					TaskParameters tp = FromUserInitiated;
					if (dia->GetAddType () != AddState::Started)
						tp |= NoAutostart;
					Core::Instance ()->AddFile (dia->GetFilename (),
							dia->GetSavePath (),
							dia->GetTags (),
							dia->GetTryLive (),
							dia->GetSelectedFiles (),
							tp);

					setActionsEnabled ();
				});
		dia->show ();
		dia->setAttribute (Qt::WA_DeleteOnClose);
	}

	void TorrentTab::handleAddMagnetTriggered ()
	{
		AddMagnetDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		Core::Instance ()->AddMagnet (dia.GetLink (),
				dia.GetPath (),
				dia.GetTags ());

		setActionsEnabled ();
	}

	namespace
	{
		bool CheckExists (const QString& torrentPath, const QDir& saveDir)
		{
			QFile torrentFile { torrentPath };
			if (!torrentFile.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open"
						<< torrentPath
						<< torrentFile.errorString ();
				return false;
			}

			const auto& torrentData = torrentFile.readAll ();
			try
			{

				libtorrent::torrent_info torrent { torrentData.constData (), torrentData.size () };
				const auto& files = torrent.files ();
				switch (files.num_files ())
				{
				case 0:
					return false;
				case 1:
					return saveDir.exists (QString::fromStdString (files.file_name (0).to_string ()));
				default:
				{
					const auto& dirName = QString::fromStdString (files.name ());
					return !dirName.isEmpty () && saveDir.exists (dirName);
				}
				}
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to parse"
						<< torrentPath
						<< e.what ();
				return false;
			}
		}
	}

	void TorrentTab::handleOpenMultipleTorrentsTriggered ()
	{
		AddMultipleTorrents dialog;
		if (dialog.exec () == QDialog::Rejected)
			return;

		TaskParameters tp = FromUserInitiated;
		if (!dialog.ShouldAddAsStarted ())
			tp |= NoAutostart;

		const auto& savePath = dialog.GetSaveDirectory ();
		const auto& openPath = dialog.GetOpenDirectory ();
		const auto& tags = dialog.GetTags ();
		const QDir saveDir { savePath };
		for (const auto& torrentName : QDir { openPath }.entryList (QStringList ("*.torrent")))
		{
			auto torrentPath = openPath;
			if (!torrentPath.endsWith ('/'))
				torrentPath += '/';
			torrentPath += torrentName;

			if (dialog.OnlyIfExists ())
			{
				bool torrentExists = CheckExists (torrentPath, saveDir);
				qDebug () << torrentName << torrentExists;
				if (!torrentExists)
					continue;
			}

			Core::Instance ()->AddFile (torrentPath, savePath, tags, false);
		}
		setActionsEnabled ();
	}

	void TorrentTab::handleIPFilterTriggered ()
	{
		IPFilterDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		Core::Instance ()->ClearFilter ();
		for (const auto& pair : dia.GetFilter ())
			Core::Instance ()->BanPeers (pair.first, pair.second);
	}

	void TorrentTab::handleCreateTorrentTriggered ()
	{
		NewTorrentWizard wizard;
		if (wizard.exec () == QDialog::Accepted)
			Core::Instance ()->MakeTorrent (wizard.GetParams ());
		setActionsEnabled ();
	}

	void TorrentTab::handleRemoveTorrentTriggered ()
	{
		auto rows = GetSelectedRows ();

		QMessageBox confirm (QMessageBox::Question,
				"LeechCraft BitTorrent",
				tr ("Do you really want to delete %n torrent(s)?", 0, rows.size ()),
				QMessageBox::Cancel);
		auto deleteTorrentsButton = confirm.addButton (tr ("&Delete"),
				QMessageBox::ActionRole);
		auto deleteTorrentsAndFilesButton = confirm.addButton (tr ("Delete with &files"),
				QMessageBox::ActionRole);
		confirm.setDefaultButton (QMessageBox::Cancel);

		confirm.exec ();

		bool withFiles = false;
		if (confirm.clickedButton () == deleteTorrentsAndFilesButton)
			withFiles = true;
		else if (confirm.clickedButton () == deleteTorrentsButton)
			; // go ahead and just delete the torrent
		else
			return;

		std::sort (rows.begin (), rows.end (), std::greater<> ());

		for (int row : rows)
			Core::Instance ()->RemoveTorrent (row, withFiles);
		Ui_.Tabs_->InvalidateSelection ();
		setActionsEnabled ();
	}

	void TorrentTab::handleResumeTriggered ()
	{
		for (int row : GetSelectedRows ())
			Core::Instance ()->ResumeTorrent (row);
		setActionsEnabled ();
	}

	void TorrentTab::handleStopTriggered ()
	{
		for (int row : GetSelectedRows ())
			Core::Instance ()->PauseTorrent (row);
		setActionsEnabled ();
	}

	namespace
	{
		template<typename T>
		std::vector<T> List2Vector (const QList<T>& list)
		{
			std::vector<T> result;
			result.reserve (list.size ());
			std::copy (list.begin (), list.end (), std::back_inserter (result));
			return result;
		}
	}

	void TorrentTab::handleMoveUpTriggered ()
	{
		const auto& sis = GetSelectedRowIndexes ();
		const auto& selections = GetSelectedRows ();

		Core::Instance ()->MoveUp (List2Vector (selections));

		auto sel = Ui_.TorrentsView_->selectionModel ();
		sel->clearSelection ();

		QItemSelection selection;
		for (const auto& si : sis)
		{
			auto sibling = si.sibling (std::max (si.row () - 1, 0), si.column ());
			selection.select (sibling, sibling);
		}

		sel->select (selection, QItemSelectionModel::Rows | QItemSelectionModel::Select);
	}

	void TorrentTab::handleMoveDownTriggered ()
	{
		const auto& sis = GetSelectedRowIndexes ();
		const auto& selections = GetSelectedRows ();

		Core::Instance ()->MoveDown (List2Vector (selections));

		auto sel = Ui_.TorrentsView_->selectionModel ();
		sel->clearSelection ();

		QItemSelection selection;
		const auto& rowCount = Core::Instance ()->rowCount ();
		for (const auto& si : sis)
		{
			auto sibling = si.sibling (std::min (si.row () + 1, rowCount - 1), 0);
			selection.select (sibling, sibling);
		}

		sel->select (selection, QItemSelectionModel::Rows | QItemSelectionModel::Select);
	}

	void TorrentTab::handleMoveToTopTriggered ()
	{
		try
		{
			Core::Instance ()->MoveToTop (List2Vector (GetSelectedRows ()));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentTab::handleMoveToBottomTriggered ()
	{
		try
		{
			Core::Instance ()->MoveToBottom (List2Vector (GetSelectedRows ()));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentTab::handleForceReannounceTriggered ()
	{
		try
		{
			for (int torrent : GetSelectedRows ())
				Core::Instance ()->ForceReannounce (torrent);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentTab::handleForceRecheckTriggered ()
	{
		try
		{
			for (int torrent : GetSelectedRows ())
				Core::Instance ()->ForceRecheck (torrent);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentTab::handleChangeTrackersTriggered ()
	{
		const auto& sis = GetSelectedRowIndexes ();

		std::vector<libtorrent::announce_entry> allTrackers;
		for (const auto& si : sis)
		{
			auto those = Core::Instance ()->GetTrackers (si.row ());
			std::copy (those.begin (), those.end (), std::back_inserter (allTrackers));
		}

		if (allTrackers.empty ())
			allTrackers = Core::Instance ()->GetTrackers (Core::Instance ()->GetCurrentTorrent ());

		std::stable_sort (allTrackers.begin (), allTrackers.end (),
				Util::ComparingBy (&libtorrent::announce_entry::url));

		auto newLast = std::unique (allTrackers.begin (), allTrackers.end (),
				[] (const libtorrent::announce_entry& l, const libtorrent::announce_entry& r)
					{ return l.url == r.url; });

		allTrackers.erase (newLast, allTrackers.end ());

		if (allTrackers.empty ())
			return;

		TrackersChanger changer;
		changer.SetTrackers (allTrackers);
		if (changer.exec () != QDialog::Accepted)
			return;

		const auto& trackers = changer.GetTrackers ();
		for (const auto& si : sis)
			Core::Instance ()->SetTrackers (trackers, si.row ());
	}

	void TorrentTab::handleMoveFilesTriggered ()
	{
		const auto currentRows = GetSelectedRows ();

		if (currentRows.empty() )
			return;

		const auto oldDirs = Util::Map (currentRows,
				[] (const int row) { return Core::Instance ()->GetTorrentDirectory (row); });

		MoveTorrentFiles mtf { oldDirs };

		if (mtf.exec () == QDialog::Rejected)
			return;

		const auto newDir = mtf.GetNewLocation ();

		XmlSettingsManager::Instance ()->setProperty ("LastMoveDirectory", newDir);

		for (auto it : Util::Views::Zip (currentRows, oldDirs))
		{
			if (it.second == newDir)
				continue;

			if (!Core::Instance ()->MoveTorrentFiles (newDir, it.first))
			{
				const auto& text = tr ("Failed to move torrent's files from %1 to %2.")
						.arg (it.second)
						.arg (newDir);
				const auto& e = Util::MakeNotification ("BitTorrent", text, Priority::Critical);
				Core::Instance ()->GetProxy ()->GetEntityManager ()->HandleEntity (e);
			}
		}

	}

	void TorrentTab::handleMakeMagnetLinkTriggered ()
	{
		QString magnet = Core::Instance ()->GetMagnetLink (GetCurrentTorrent ());
		if (magnet.isEmpty ())
			return;

		QInputDialog *dia = new QInputDialog ();
		dia->setWindowTitle ("LeechCraft");
		dia->setLabelText (tr ("Magnet link:"));
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->setInputMode (QInputDialog::TextInput);
		dia->setTextValue (magnet);
		dia->resize (700, dia->height ());
		dia->show ();
	}
}
}

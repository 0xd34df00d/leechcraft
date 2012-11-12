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

#include "torrenttab.h"
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QSortFilterProxyModel>
#include <util/tags/tagscompleter.h>
#include <util/util.h>
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

namespace LeechCraft
{
namespace Plugins
{
namespace BitTorrent
{
	TorrentTab::TorrentTab (const TabClassInfo& tc, QObject *mt)
	: TC_ (tc)
	, ParentMT_ (mt)
	, Toolbar_ (new QToolBar ("BitTorrent"))
	, ViewFilter_ (new TabViewProxyModel (this))
	{
		Ui_.setupUi (this);

		ViewFilter_->setDynamicSortFilter (true);
		ViewFilter_->setSourceModel (Core::Instance ());

		Ui_.TorrentsView_->setModel (ViewFilter_);
		connect (Ui_.TorrentsView_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleTorrentSelected (QModelIndex)));

		const auto& fm = Ui_.TorrentsView_->fontMetrics ();
		QHeaderView *header = Ui_.TorrentsView_->header ();
		header->resizeSection (Core::Columns::ColumnID, fm.width ("999"));
		header->resizeSection (Core::Columns::ColumnName, fm.width ("boardwalk.empire.s03e02.hdtv.720p.ac3.rus.eng.novafilm.tv.mkv") * 1.3);

		new Util::TagsCompleter (Ui_.SearchLine_);
		Ui_.SearchLine_->AddSelector ();
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
				SLOT (on_OpenTorrent__triggered ()));

		CreateTorrent_ = new QAction (tr ("Create torrent..."), Toolbar_);
		CreateTorrent_->setProperty ("ActionIcon", "document-new");
		connect (CreateTorrent_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_CreateTorrent__triggered ()));

		OpenMultipleTorrents_ = new QAction (tr ("Open multiple torrents..."), Toolbar_);
		OpenMultipleTorrents_->setProperty ("ActionIcon", "document-open-folder");
		connect (OpenMultipleTorrents_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_OpenMultipleTorrents__triggered ()));

		IPFilter_ = new QAction (tr ("IP filter..."), Toolbar_);
		IPFilter_->setProperty ("ActionIcon", "view-filter");
		connect (IPFilter_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_IPFilter__triggered ()));

		RemoveTorrent_ = new QAction (tr ("Remove"), Toolbar_);
		RemoveTorrent_->setShortcut (tr ("Del"));
		RemoveTorrent_->setProperty ("ActionIcon", "list-remove");
		connect (RemoveTorrent_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_RemoveTorrent__triggered ()));

		Resume_ = new QAction (tr ("Resume"), Toolbar_);
		Resume_->setShortcut (tr ("R"));
		Resume_->setProperty ("ActionIcon", "media-playback-start");
		connect (Resume_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_Resume__triggered ()));

		Stop_ = new QAction (tr ("Pause"), Toolbar_);
		Stop_->setShortcut (tr ("S"));
		Stop_->setProperty ("ActionIcon", "media-playback-pause");
		connect (Stop_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_Stop__triggered ()));

		MoveUp_ = new QAction (tr ("Move up"), Toolbar_);
		MoveUp_->setShortcut (Qt::CTRL + Qt::Key_Up);
		MoveUp_->setProperty ("ActionIcon", "go-up");
		connect (MoveUp_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveUp__triggered ()));

		MoveDown_ = new QAction (tr ("Move down"), Toolbar_);
		MoveDown_->setShortcut (Qt::CTRL + Qt::Key_Down);
		MoveDown_->setProperty ("ActionIcon", "go-down");
		connect (MoveDown_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveDown__triggered ()));

		MoveToTop_ = new QAction (tr ("Move to top"), Toolbar_);
		MoveToTop_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
		MoveToTop_->setProperty ("ActionIcon", "go-top");
		connect (MoveToTop_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveToTop__triggered ()));

		MoveToBottom_ = new QAction (tr ("Move to bottom"), Toolbar_);
		MoveToBottom_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
		MoveToBottom_->setProperty ("ActionIcon", "go-bottom");
		connect (MoveToBottom_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveToBottom__triggered ()));

		ForceReannounce_ = new QAction (tr ("Reannounce"), Toolbar_);
		ForceReannounce_->setShortcut (tr ("F"));
		ForceReannounce_->setProperty ("ActionIcon", "network-wireless");
		connect (ForceReannounce_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_ForceReannounce__triggered ()));

		ForceRecheck_ = new QAction (tr ("Recheck"), Toolbar_);
		ForceRecheck_->setProperty ("ActionIcon", "tools-check-spelling");
		connect (ForceRecheck_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_ForceRecheck__triggered ()));

		MoveFiles_ = new QAction (tr ("Move files..."), Toolbar_);
		MoveFiles_->setShortcut (tr ("M"));
		MoveFiles_->setProperty ("ActionIcon", "transform-move");
		connect (MoveFiles_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveFiles__triggered ()));

		ChangeTrackers_ = new QAction (tr ("Change trackers..."), Toolbar_);
		ChangeTrackers_->setShortcut (tr ("C"));
		ChangeTrackers_->setProperty ("ActionIcon", "view-media-playlist");
		connect (ChangeTrackers_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_ChangeTrackers__triggered ()));

		MakeMagnetLink_ = new QAction (tr ("Make magnet link..."), Toolbar_);
		MakeMagnetLink_->setProperty ("ActionIcon", "insert-link");
		connect (MakeMagnetLink_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_MakeMagnetLink__triggered ()));

		Toolbar_->addAction (OpenTorrent_);
		Toolbar_->addAction (RemoveTorrent_);
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

		/*
		QMenu *contextMenu = new QMenu (tr ("Torrents actions"));
		contextMenu->addAction (RemoveTorrent_.get ());
		contextMenu->addSeparator ();
		contextMenu->addAction (MoveUp_.get ());
		contextMenu->addAction (MoveDown_.get ());
		contextMenu->addAction (MoveToTop_.get ());
		contextMenu->addAction (MoveToBottom_.get ());
		contextMenu->addSeparator ();
		contextMenu->addAction (ForceReannounce_.get ());
		contextMenu->addAction (ForceRecheck_.get ());
		contextMenu->addAction (MoveFiles_.get ());
		contextMenu->addAction (ChangeTrackers_.get ());
		contextMenu->addAction (MakeMagnetLink_.get ());
		Core::Instance ()->SetMenu (contextMenu);
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

	int TorrentTab::GetCurrentTorrent () const
	{
		return ViewFilter_->mapToSource (Ui_.TorrentsView_->currentIndex ()).row ();
	}

	QList<int> TorrentTab::GetSelectedRows () const
	{
		QList<int> result;
		Q_FOREACH (const auto& idx, GetSelectedRowIndexes ())
			result << idx.row ();
		return result;
	}

	QModelIndexList TorrentTab::GetSelectedRowIndexes () const
	{
		QModelIndexList result;
		Q_FOREACH (const auto& idx, Ui_.TorrentsView_->selectionModel ()->selectedRows ())
			result << ViewFilter_->mapToSource (idx);
		return result;
	}

	void TorrentTab::handleTorrentSelected (const QModelIndex& index)
	{
		Ui_.Tabs_->SetCurrentIndex (ViewFilter_->mapToSource (index).row ());
	}

	void TorrentTab::setActionsEnabled ()
	{
	}

	void TorrentTab::on_OpenTorrent__triggered ()
	{
		AddTorrent dia;
		if (dia.exec () == QDialog::Rejected)
			return;

		const auto& filename = dia.GetFilename ();
		const auto& path = dia.GetSavePath ();
		bool tryLive = dia.GetTryLive ();
		const auto& files = dia.GetSelectedFiles ();
		const auto& tags = dia.GetTags ();

		TaskParameters tp = FromUserInitiated;
		if (dia.GetAddType () != Core::Started)
			tp |= NoAutostart;
		Core::Instance ()->AddFile (filename,
				path,
				tags,
				tryLive,
				files,
				tp);

		setActionsEnabled ();
	}

	void TorrentTab::on_OpenMultipleTorrents__triggered ()
	{
		AddMultipleTorrents dialog;
		std::unique_ptr<Util::TagsCompleter> completer (new Util::TagsCompleter (dialog.GetEdit (), this));
		dialog.GetEdit ()->AddSelector ();

		if (dialog.exec () == QDialog::Rejected)
			return;

		TaskParameters tp = FromUserInitiated;
		if (dialog.GetAddType () != Core::Started)
			tp |= NoAutostart;

		QString savePath = dialog.GetSaveDirectory (),
				openPath = dialog.GetOpenDirectory ();
		QDir dir (openPath);
		QStringList names = dir.entryList (QStringList ("*.torrent"));
		QStringList tags = dialog.GetTags ();
		for (int i = 0; i < names.size (); ++i)
		{
			QString name = openPath;
			if (!name.endsWith ('/'))
				name += '/';
			name += names.at (i);
			Core::Instance ()->AddFile (name, savePath, tags, false);
		}
		setActionsEnabled ();
	}

	void TorrentTab::on_IPFilter__triggered ()
	{
		IPFilterDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		Core::Instance ()->ClearFilter ();
		const auto& filter = dia.GetFilter ();
		Q_FOREACH (const auto& pair, filter)
			Core::Instance ()->BanPeers (pair.first, pair.second);
	}

	void TorrentTab::on_CreateTorrent__triggered ()
	{
		NewTorrentWizard wizard;
		if (wizard.exec () == QDialog::Accepted)
			Core::Instance ()->MakeTorrent (wizard.GetParams ());
		setActionsEnabled ();
	}

	void TorrentTab::on_RemoveTorrent__triggered ()
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

		int roptions = libtorrent::session::none;
		if (confirm.clickedButton () == deleteTorrentsAndFilesButton)
			roptions |= libtorrent::session::delete_files;
		else if (confirm.clickedButton () == deleteTorrentsButton)
			;// do nothing
		else
			return;

		std::sort (rows.begin (), rows.end (), std::greater<int> ());

		Q_FOREACH (int row, rows)
			Core::Instance ()->RemoveTorrent (row, roptions);
		Ui_.Tabs_->InvalidateSelection ();
		setActionsEnabled ();
	}

	void TorrentTab::on_Resume__triggered ()
	{
		Q_FOREACH (int row, GetSelectedRows ())
			Core::Instance ()->ResumeTorrent (row);
		setActionsEnabled ();
	}

	void TorrentTab::on_Stop__triggered ()
	{
		Q_FOREACH (int row, GetSelectedRows ())
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

	void TorrentTab::on_MoveUp__triggered ()
	{
		const auto& sis = GetSelectedRowIndexes ();
		const auto& selections = GetSelectedRows ();

		Core::Instance ()->MoveUp (List2Vector (selections));

		QItemSelectionModel *sel = qobject_cast<QItemSelectionModel*> (sender ()->
				property ("ItemSelectionModel").value<QObject*> ());

		if (sel)
			sel->clearSelection ();

		QItemSelection selection;
		Q_FOREACH (QModelIndex si, sis)
		{
			QModelIndex sibling = si.sibling (si.row () - 1, si.column ());
			selection.select (sibling, sibling);
		}

		if (sel)
			sel->select (selection, QItemSelectionModel::Rows |
					QItemSelectionModel::SelectCurrent);
	}

	void TorrentTab::on_MoveDown__triggered ()
	{
		const auto& sis = GetSelectedRowIndexes ();
		const auto& selections = GetSelectedRows ();

		Core::Instance ()->MoveDown (List2Vector (selections));

		QItemSelectionModel *sel = qobject_cast<QItemSelectionModel*> (sender ()->
				property ("ItemSelectionModel").value<QObject*> ());

		if (sel)
			sel->clearSelection ();

		QItemSelection selection;
		Q_FOREACH (QModelIndex si, sis)
		{
			QModelIndex sibling = si.sibling (si.row () + 1, 0);
			selection.select (sibling, sibling);
		}

		if (sel)
			sel->select (selection, QItemSelectionModel::Rows |
					QItemSelectionModel::SelectCurrent);
	}

	void TorrentTab::on_MoveToTop__triggered ()
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

	void TorrentTab::on_MoveToBottom__triggered ()
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

	void TorrentTab::on_ForceReannounce__triggered ()
	{
		try
		{
			Q_FOREACH (int torrent, GetSelectedRows ())
				Core::Instance ()->ForceReannounce (torrent);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentTab::on_ForceRecheck__triggered ()
	{
		try
		{
			Q_FOREACH (int torrent, GetSelectedRows ())
				Core::Instance ()->ForceRecheck (torrent);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentTab::on_ChangeTrackers__triggered ()
	{
		const auto& sis = GetSelectedRowIndexes ();

		std::vector<libtorrent::announce_entry> allTrackers;
		Q_FOREACH (const auto& si, sis)
		{
			auto those = Core::Instance ()->GetTrackers (si.row ());
			std::copy (those.begin (), those.end (), std::back_inserter (allTrackers));
		}

		if (allTrackers.empty ())
			allTrackers = Core::Instance ()->
					GetTrackers (Core::Instance ()->
							GetCurrentTorrent ());

		std::stable_sort (allTrackers.begin (), allTrackers.end (),
				[] (const libtorrent::announce_entry& l, const libtorrent::announce_entry& r)
					{ return l.url < r.url; });

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
		Q_FOREACH (const auto& si, sis)
			Core::Instance ()->SetTrackers (trackers, si.row ());
	}

	void TorrentTab::on_MoveFiles__triggered ()
	{
		const int current = GetCurrentTorrent ();
		QString oldDir = Core::Instance ()->GetTorrentDirectory (current);
		MoveTorrentFiles mtf (oldDir);
		if (mtf.exec () == QDialog::Rejected)
			return;
		QString newDir = mtf.GetNewLocation ();
		if (oldDir == newDir)
			return;

		if (!Core::Instance ()->MoveTorrentFiles (newDir, current))
		{
			QString text = tr ("Failed to move torrent's files from %1 to %2")
					.arg (oldDir)
					.arg (newDir);
			const auto& e = Util::MakeNotification ("BitTorrent", text, PCritical_);
			Core::Instance ()->GetProxy ()->GetEntityManager ()->HandleEntity (e);
		}
	}

	void TorrentTab::on_MakeMagnetLink__triggered ()
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
}

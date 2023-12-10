/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addtorrent.h"
#include <filesystem>
#include <optional>
#include <QFileDialog>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <libtorrent/torrent_info.hpp>
#include <util/util.h>
#include <util/gui/util.h>
#include <util/sll/prelude.h>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "addtorrentfilesmodel.h"
#include "xmlsettingsmanager.h"

namespace LC::BitTorrent
{
	AddTorrent::AddTorrent (QWidget *parent)
	: QDialog { parent }
	, FilesModel_ { new AddTorrentFilesModel { this } }
	, ProxyModel_ { new QSortFilterProxyModel { this } }
	{
		Ui_.setupUi (this);

		new Util::TagsCompleter { Ui_.TagsEdit_ };
		Ui_.TagsEdit_->AddSelector ();

		ProxyModel_->setSourceModel (FilesModel_);
		ProxyModel_->setSortRole (AddTorrentFilesModel::RoleSort);
		ProxyModel_->setDynamicSortFilter (true);

		Ui_.FilesView_->setModel (ProxyModel_);
		Ui_.FilesView_->sortByColumn (0, Qt::AscendingOrder);

		Ui_.OK_->setEnabled (false);

		const auto setOkEnabled = [this]
		{
			Ui_.OK_->setEnabled (QFileInfo { Ui_.TorrentFile_->text () }.isReadable () &&
					QFileInfo::exists (Ui_.Destination_->text ()));
		};
		connect (Ui_.TorrentFile_,
				&QLineEdit::textChanged,
				setOkEnabled);
		connect (Ui_.Destination_,
				&QLineEdit::textChanged,
				setOkEnabled);
		connect (Ui_.Destination_,
				&QLineEdit::textChanged,
				this,
				&AddTorrent::UpdateSpaceDisplay);

		Ui_.Destination_->setText (XmlSettingsManager::Instance ().property ("LastSaveDirectory").toString ());

		auto markMenu = new QMenu { Ui_.MarkMenuButton_ };
		markMenu->addActions ({
				Ui_.MarkAll_,
				Ui_.UnmarkAll_,
				Ui_.MarkSelected_,
				Ui_.UnmarkSelected_,
				Ui_.MarkExisting_,
				Ui_.MarkMissing_
			});
		Ui_.MarkMenuButton_->setMenu (markMenu);

		const auto header = Ui_.FilesView_->header ();
		const auto& fm = fontMetrics ();
		header->resizeSection (0, fm.horizontalAdvance (QStringLiteral ("Thisisanaveragetorrentcontainedfilename,ormaybeevenbiggerthanthat!")));
		header->resizeSection (1, fm.horizontalAdvance (QStringLiteral ("_999.9 MB_")));
		header->setStretchLastSection (true);

		connect (Ui_.ExpandAll_,
				&QPushButton::released,
				Ui_.FilesView_,
				&QTreeView::expandAll);
		connect (Ui_.CollapseAll_,
				&QPushButton::released,
				Ui_.FilesView_,
				&QTreeView::collapseAll);

		connect (Ui_.TorrentBrowse_,
				&QPushButton::released,
				this,
				&AddTorrent::BrowseForTorrent);
		connect (Ui_.DestinationBrowse_,
				&QPushButton::released,
				this,
				&AddTorrent::BrowseForDestination);

		connect (Ui_.MarkAll_,
				&QAction::triggered,
				FilesModel_,
				&AddTorrentFilesModel::MarkAll);
		connect (Ui_.UnmarkAll_,
				&QAction::triggered,
				FilesModel_,
				&AddTorrentFilesModel::UnmarkAll);
		connect (Ui_.MarkExisting_,
				&QAction::triggered,
				[this] { MarkExisting (Qt::Checked, Qt::Unchecked); });
		connect (Ui_.MarkMissing_,
				&QAction::triggered,
				[this] { MarkExisting (Qt::Unchecked, Qt::Checked); });

		const auto selectedAsFilesModel = [this]
		{
			return Util::Map (Ui_.FilesView_->selectionModel ()->selectedRows (),
					[this] (const QModelIndex& idx) { return ProxyModel_->mapToSource (idx); });
		};
		connect (Ui_.MarkSelected_,
				&QAction::triggered,
				[=, this] { FilesModel_->MarkIndexes (selectedAsFilesModel ()); });
		connect (Ui_.UnmarkSelected_,
				&QAction::triggered,
				[=, this] { FilesModel_->UnmarkIndexes (selectedAsFilesModel ()); });
	}

	void AddTorrent::SetFilename (const QString& filename)
	{
		if (filename.isEmpty ())
			return;

		XmlSettingsManager::Instance ().setProperty ("LastTorrentDirectory",
				QFileInfo (filename).absolutePath ());
		Ui_.TorrentFile_->setText (filename);

		ParseBrowsed ();
	}

	void AddTorrent::SetSavePath (const QString& path)
	{
		Ui_.Destination_->setText (path);
	}

	QString AddTorrent::GetFilename () const
	{
		return Ui_.TorrentFile_->text ();
	}

	QString AddTorrent::GetSavePath () const
	{
		return Ui_.Destination_->text ();
	}

	bool AddTorrent::GetTryLive () const
	{
		return Ui_.TryLive_->checkState () == Qt::Checked;
	}

	QVector<bool> AddTorrent::GetSelectedFiles () const
	{
		return FilesModel_->GetSelectedFiles ();
	}

	AddState AddTorrent::GetAddType () const
	{
		switch (Ui_.AddTypeBox_->currentIndex ())
		{
		case 0:
			return AddState::Started;
		case 1:
			return AddState::Paused;
		default:
			return AddState::Started;
		}
	}

	void AddTorrent::SetTags (const QStringList& ids)
	{
		Ui_.TagsEdit_->setText (GetProxyHolder ()->GetTagsManager ()->JoinIDs (ids));
	}

	QStringList AddTorrent::GetTags () const
	{
		return GetProxyHolder ()->GetTagsManager ()->SplitToIDs (Ui_.TagsEdit_->text ());
	}

	namespace
	{
		struct SpaceInfo
		{
			uint64_t Available_;
			int UsedPercentage_;
		};

		std::optional<SpaceInfo> GetAvailableSpace (const QString& path)
		{
			try
			{
				const auto space = std::filesystem::space (path.toStdString ());
				const int freePercentage = 100 * space.available / space.capacity;
				return SpaceInfo { .Available_ = space.available, .UsedPercentage_ = 100 - freePercentage };
			}
			catch (...)
			{
				return {};
			}
		}
	}

	void AddTorrent::UpdateSpaceDisplay ()
	{
		if (const auto space = GetAvailableSpace (GetSavePath ()))
		{
			Ui_.AvailSpaceLabel_->setText (tr ("%1 free").arg (Util::MakePrettySize (space->Available_)));
			Ui_.AvailSpaceBar_->setValue (space->UsedPercentage_);
			Ui_.AvailSpaceBar_->show ();
		}
		else
		{
			Ui_.AvailSpaceLabel_->setText (tr ("unknown"));
			Ui_.AvailSpaceBar_->hide ();
		}
	}

	void AddTorrent::BrowseForTorrent ()
	{
		const auto& filename = QFileDialog::getOpenFileName (this,
				tr ("Select torrent file"),
				XmlSettingsManager::Instance ().property ("LastTorrentDirectory").toString (),
				tr ("Torrents (*.torrent);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		XmlSettingsManager::Instance ().setProperty ("LastTorrentDirectory",
				QFileInfo { filename }.absolutePath ());
		Ui_.TorrentFile_->setText (filename);

		ParseBrowsed ();
	}

	void AddTorrent::BrowseForDestination ()
	{
		const auto& dir = QFileDialog::getExistingDirectory (this,
				tr ("Select save directory"),
				Ui_.Destination_->text ());
		if (dir.isEmpty ())
			return;

		XmlSettingsManager::Instance ().setProperty ("LastSaveDirectory", dir);
		Ui_.Destination_->setText (dir);
	}

	void AddTorrent::MarkExisting (Qt::CheckState ifExists, Qt::CheckState ifNotExists)
	{
		auto rootPath = GetSavePath ();
		if (!rootPath.endsWith ('/'))
			rootPath += '/';

		QList<QModelIndex> queue { QModelIndex {} };
		while (!queue.isEmpty ())
		{
			const auto& idx = queue.takeFirst ();

			if (const auto rc = FilesModel_->rowCount (idx))
				for (int i = 0; i < rc; ++i)
					queue << FilesModel_->index (i, 0, idx);
			else
			{
				const auto& subpath = idx.data (AddTorrentFilesModel::RoleFullPath).toString ();

				const bool exists = QFile::exists (rootPath + subpath);
				FilesModel_->setData (idx, exists ? ifExists : ifNotExists, Qt::CheckStateRole);
			}
		}
	}

	void AddTorrent::ParseBrowsed ()
	{
		const auto& filename = Ui_.TorrentFile_->text ();

		QFile file { filename };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< filename
					<< file.errorString ();
			return;
		}

		std::optional<libtorrent::torrent_info> maybeInfo;
		try
		{
			const auto& contents = file.readAll ();
			maybeInfo.emplace (contents.constData (), contents.size ());
		}
		catch (const std::exception& e)
		{
			Ui_.OK_->setEnabled (false);
			QMessageBox::critical (this,
					QStringLiteral ("LeechCraft"),
					tr ("Unable to parse %1: %2.")
						.arg (Util::FormatName (filename), e.what ()));
			return;
		}

		const auto& info = *maybeInfo;

		Ui_.Size_->setText (Util::MakePrettySize (info.total_size ()));

		const auto setText = [this] (QLabel *edit, const QString& text)
		{
			edit->setText (text);

			Ui_.InfoLayout_->labelForField (edit)->setVisible (!text.isEmpty ());
			edit->setVisible (!text.isEmpty ());
		};

		setText (Ui_.TrackerURL_,
				info.trackers ().empty () ?
					QString {} :
					QString::fromStdString (info.trackers ().at (0).url));
		setText (Ui_.Creator_, QString::fromStdString (info.creator ()));
		setText (Ui_.Comment_, QString::fromStdString (info.comment ()));
		setText (Ui_.Date_,
				info.creation_date () ?
					QDateTime::fromSecsSinceEpoch (info.creation_date ()).toString () :
					QString {});

		QList<AddTorrentFilesModel::FileEntry> fileEntries;
		const auto& torrentFiles = info.files ();
		for (int i = 0; i < torrentFiles.num_files (); ++i)
			fileEntries.push_back ({ torrentFiles.file_path (i), torrentFiles.file_size (i) });
		FilesModel_->ResetFiles (fileEntries);

		Ui_.FilesView_->expandAll ();
	}
}

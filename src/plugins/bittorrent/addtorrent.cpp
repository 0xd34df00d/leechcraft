/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addtorrent.h"
#include <filesystem>
#include <libtorrent/announce_entry.hpp>
#include <QFileDialog>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "addtorrentfilesmodel.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC
{
namespace BitTorrent
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

		connect (this,
				SIGNAL (on_TorrentFile__textChanged ()),
				this,
				SLOT (setOkEnabled ()));
		connect (this,
				SIGNAL (on_Destination__textChanged ()),
				this,
				SLOT (setOkEnabled ()));
		connect (this,
				SIGNAL (on_Destination__textChanged ()),
				this,
				SLOT (updateAvailableSpace ()));

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
		header->resizeSection (0, fm.horizontalAdvance ("Thisisanaveragetorrentcontainedfilename,ormaybeevenbiggerthanthat!"));
		header->resizeSection (1, fm.horizontalAdvance ("_999.9 MB_"));
		header->setStretchLastSection (true);

		connect (Ui_.ExpandAll_,
				SIGNAL (released ()),
				Ui_.FilesView_,
				SLOT (expandAll ()));
		connect (Ui_.CollapseAll_,
				SIGNAL (released ()),
				Ui_.FilesView_,
				SLOT (collapseAll ()));
	}

	void AddTorrent::Reinit ()
	{
		FilesModel_->Clear ();
		Ui_.TorrentFile_->setText ("");
		Ui_.TrackerURL_->setText (tr ("<unknown>"));
		Ui_.Size_->setText (tr ("<unknown>"));
		Ui_.Creator_->setText (tr ("<unknown>"));
		Ui_.Comment_->setText (tr ("<unknown>"));
		Ui_.Date_->setText (tr ("<unknown>"));

		const auto& dir = XmlSettingsManager::Instance ()->
				property ("LastSaveDirectory").toString ();
		Ui_.Destination_->setText (dir);

		updateAvailableSpace ();
	}

	void AddTorrent::SetFilename (const QString& filename)
	{
		if (filename.isEmpty ())
			return;

		Reinit ();

		XmlSettingsManager::Instance ()->setProperty ("LastTorrentDirectory",
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
		auto tm = Core::Instance ()->GetProxy ()->GetTagsManager ();
		Ui_.TagsEdit_->setText (tm->Join (tm->GetTags (ids)));
	}

	QStringList AddTorrent::GetTags () const
	{
		auto tm = Core::Instance ()->GetProxy ()->GetTagsManager ();

		QStringList result;
		for (const auto& tag : tm->Split (Ui_.TagsEdit_->text ()))
			result << tm->GetID (tag);
		return result;
	}

	Util::TagsLineEdit* AddTorrent::GetEdit ()
	{
		return Ui_.TagsEdit_;
	}

	void AddTorrent::setOkEnabled ()
	{
		Ui_.OK_->setEnabled (QFileInfo (Ui_.TorrentFile_->text ()).isReadable () &&
				QFileInfo (Ui_.Destination_->text ()).exists ());
	}

	void AddTorrent::updateAvailableSpace ()
	{
		const auto& pair = GetAvailableSpaceInDestination ();
		const quint64 availableSpace = pair.first;
		const quint64 totalSpace = pair.second;

		if (availableSpace != static_cast<quint64> (-1))
		{
			Ui_.AvailSpaceLabel_->setText (tr ("%1 free").arg (Util::MakePrettySize (availableSpace)));
			Ui_.AvailSpaceBar_->show ();
			Ui_.AvailSpaceBar_->setValue (100 - 100 * availableSpace / totalSpace);
		}
		else
		{
			Ui_.AvailSpaceLabel_->setText (tr ("unknown"));
			Ui_.AvailSpaceBar_->hide ();
		}
	}

	void AddTorrent::on_TorrentBrowse__released ()
	{
		const auto& filename = QFileDialog::getOpenFileName (this,
				tr ("Select torrent file"),
				XmlSettingsManager::Instance ()->property ("LastTorrentDirectory").toString (),
				tr ("Torrents (*.torrent);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		Reinit ();

		XmlSettingsManager::Instance ()->setProperty ("LastTorrentDirectory",
				QFileInfo (filename).absolutePath ());
		Ui_.TorrentFile_->setText (filename);

		ParseBrowsed ();
	}

	void AddTorrent::on_DestinationBrowse__released ()
	{
		const auto& dir = QFileDialog::getExistingDirectory (this,
				tr ("Select save directory"),
				Ui_.Destination_->text (),
				{});
		if (dir.isEmpty ())
			return;

		XmlSettingsManager::Instance ()->setProperty ("LastSaveDirectory", dir);
		Ui_.Destination_->setText (dir);
	}

	void AddTorrent::on_MarkAll__triggered ()
	{
		FilesModel_->MarkAll ();
	}

	void AddTorrent::on_UnmarkAll__triggered ()
	{
		FilesModel_->UnmarkAll ();
	}

	void AddTorrent::on_MarkSelected__triggered ()
	{
		const auto& indices = Util::Map (Ui_.FilesView_->selectionModel ()->selectedRows (),
				[this] (const QModelIndex& idx) { return ProxyModel_->mapToSource (idx); });
		FilesModel_->MarkIndexes (indices);
	}

	void AddTorrent::on_UnmarkSelected__triggered ()
	{
		const auto& indices = Util::Map (Ui_.FilesView_->selectionModel ()->selectedRows (),
				[this] (const QModelIndex& idx) { return ProxyModel_->mapToSource (idx); });
		FilesModel_->UnmarkIndexes (indices);
	}

	void AddTorrent::on_MarkExisting__triggered ()
	{
		MarkExisting ([] (bool exists) { return exists ? Qt::Checked : Qt::Unchecked; });
	}

	void AddTorrent::on_MarkMissing__triggered ()
	{
		MarkExisting ([] (bool exists) { return exists ? Qt::Unchecked : Qt::Checked; });
	}

	template<typename T>
	void AddTorrent::MarkExisting (T bool2mark)
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
				FilesModel_->setData (idx, bool2mark (exists), Qt::CheckStateRole);
			}
		}
	}

	void AddTorrent::ParseBrowsed ()
	{
		const auto& filename = Ui_.TorrentFile_->text ();
		const auto& info = Core::Instance ()->GetTorrentInfo (filename);
		Ui_.OK_->setEnabled (info.is_valid ());
		if (!info.is_valid ())
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Looks like %1 is not a valid torrent file.")
						.arg ("<em>" + filename + "</em>"));
			return;
		}

		if (info.trackers ().size ())
			Ui_.TrackerURL_->setText (QString::fromStdString (info.trackers ().at (0).url));
		else
			Ui_.TrackerURL_->setText (tr ("<no trackers>"));
		Ui_.Size_->setText (Util::MakePrettySize (info.total_size ()));

		QString creator = QString::fromUtf8 (info.creator ().c_str ()),
				comment = QString::fromUtf8 (info.comment ().c_str ());

		QString date;
		if (const auto maybeDate = info.creation_date ())
			date = QDateTime::fromSecsSinceEpoch (maybeDate).toString ();

		if (!creator.isEmpty () && !creator.isNull ())
			Ui_.Creator_->setText (creator);
		else
			Ui_.Creator_->setText ("<>");
		if (!comment.isEmpty () && !comment.isNull ())
			Ui_.Comment_->setText (comment);
		else
			Ui_.Comment_->setText ("<>");
		if (!date.isEmpty () && !date.isNull ())
			Ui_.Date_->setText (date);
		else
			Ui_.Date_->setText ("<>");

		QList<AddTorrentFilesModel::FileEntry> fileEntries;
		const auto& torrentFiles = info.files ();
		for (int i = 0; i < torrentFiles.num_files (); ++i)
			fileEntries.push_back ({ torrentFiles.file_path (i), torrentFiles.file_size (i) });
		FilesModel_->ResetFiles (fileEntries);

		Ui_.FilesView_->expandAll ();
	}

	QPair<quint64, quint64> AddTorrent::GetAvailableSpaceInDestination ()
	{
		try
		{
			const auto space = std::filesystem::space (GetSavePath ().toStdString ());
			return qMakePair<quint64, quint64> (space.available, space.capacity);
		}
		catch (...)
		{
			return qMakePair<quint64, quint64> (-1, -1);
		}
	}
}
}

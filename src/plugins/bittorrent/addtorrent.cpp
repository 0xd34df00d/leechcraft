/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <QHeaderView>
#include <QFileDialog>
#include <plugininterface/util.h>
#include "addtorrent.h"
#include "torrentfilesmodel.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			AddTorrent::AddTorrent (QWidget *parent)
			: QDialog (parent)
			{
				setupUi (this);
				FilesModel_ = new TorrentFilesModel (true, this);
				FilesView_->header ()->setStretchLastSection (true);
				FilesView_->setModel (FilesModel_);
				OK_->setEnabled (false);
				connect (this,
						SIGNAL (on_TorrentFile__textChanged ()),
						this,
						SLOT (setOkEnabled ()));
				connect (this,
						SIGNAL (on_Destination__textChanged ()),
						this,
						SLOT (setOkEnabled ()));
			
				QString dir = XmlSettingsManager::Instance ()->property ("LastSaveDirectory").toString ();
				Destination_->setText (dir);
			
				QFontMetrics fm = fontMetrics ();
				QHeaderView *header = FilesView_->header ();
				header->resizeSection (0, fm.width ("Thisisanaveragetorrentcontainedfilename,ormaybeevenbiggerthanthat!"));
				header->resizeSection (1, fm.width ("_999.9 MB_"));
			}
			
			void AddTorrent::Reinit ()
			{
				FilesModel_->Clear ();
				TorrentFile_->setText ("");
				TrackerURL_->setText (tr ("<unknown>"));
				Size_->setText (tr ("<unknown>"));
				Creator_->setText (tr ("<unknown>"));
				Comment_->setText (tr ("<unknown>"));
				Date_->setText (tr ("<unknown>"));
			}
			
			void AddTorrent::SetFilename (const QString& filename)
			{
				if (filename.isEmpty ())
					return;
			
				Reinit ();
			
				XmlSettingsManager::Instance ()->
					setProperty ("LastTorrentDirectory", QFileInfo (filename).absolutePath ());
				TorrentFile_->setText (filename);
			
				ParseBrowsed ();
			}
			
			void AddTorrent::SetSavePath (const QString& path)
			{
				Destination_->setText (path);
			}
			
			QString AddTorrent::GetFilename () const
			{
				return TorrentFile_->text ();
			}
			
			QString AddTorrent::GetSavePath () const
			{
				return Destination_->text ();
			}

			bool AddTorrent::GetTryLive () const
			{
				return TryLive_->checkState () == Qt::Checked;
			}
			
			QVector<bool> AddTorrent::GetSelectedFiles () const
			{
				return FilesModel_->GetSelectedFiles ();
			}
			
			Core::AddType AddTorrent::GetAddType () const
			{
				switch (AddTypeBox_->currentIndex ())
				{
					case 0:
						return Core::Started;
					case 1:
						return Core::Paused;
					default:
						return Core::Started;
				}
			}

			void AddTorrent::SetTags (const QStringList& ids)
			{
				QStringList tags;
				Q_FOREACH (QString id, ids)
					tags << Core::Instance ()->GetProxy ()->
						GetTagsManager ()->GetTag (id);
				TagsEdit_->setText (Core::Instance ()->
						GetProxy ()->GetTagsManager ()->Join (tags));
			}
			
			QStringList AddTorrent::GetTags () const
			{
				QStringList tags = Core::Instance ()->GetProxy ()->
					GetTagsManager ()->Split (TagsEdit_->text ());
				QStringList result;
				Q_FOREACH (QString tag, tags)
					result << Core::Instance ()->GetProxy ()->
						GetTagsManager ()->GetID (tag);
				return result;
			}
			
			Util::TagsLineEdit* AddTorrent::GetEdit ()
			{
				return TagsEdit_;
			}
			
			void AddTorrent::setOkEnabled ()
			{
				OK_->setEnabled (QFileInfo (TorrentFile_->text ()).isReadable () &&
						QFileInfo (Destination_->text ()).exists ());
			}
			
			void AddTorrent::on_TorrentBrowse__released ()
			{
				  QString filename = QFileDialog::getOpenFileName (this,
							tr ("Select torrent file"),
							XmlSettingsManager::Instance ()->
								property ("LastTorrentDirectory").toString (),
							tr ("Torrents (*.torrent);;All files (*.*)"));
				  if (filename.isEmpty ())
					return;
			
				Reinit ();
			
				XmlSettingsManager::Instance ()->setProperty ("LastTorrentDirectory",
						QFileInfo (filename).absolutePath ());
				TorrentFile_->setText (filename);
			
				ParseBrowsed ();
			}
			
			void AddTorrent::on_DestinationBrowse__released ()
			{
				QString dir = QFileDialog::getExistingDirectory (this,
						tr ("Select save directory"),
						Destination_->text (),
						0);
				if (dir.isEmpty ())
					return;
			
				XmlSettingsManager::Instance ()->setProperty ("LastSaveDirectory", dir);
				Destination_->setText (dir);
			}
			
			void AddTorrent::on_MarkAll__released ()
			{
				FilesModel_->MarkAll ();
			}
			
			void AddTorrent::on_UnmarkAll__released ()
			{
				FilesModel_->UnmarkAll ();
			}
			
			void AddTorrent::on_MarkSelected__released ()
			{
				FilesModel_->MarkIndexes (FilesView_->selectionModel ()->selectedRows ());
			}
			
			void AddTorrent::on_UnmarkSelected__released ()
			{
				FilesModel_->UnmarkIndexes (FilesView_->selectionModel ()->selectedRows ());
			}
			
			void AddTorrent::ParseBrowsed ()
			{
				QString filename = TorrentFile_->text ();
				libtorrent::torrent_info info = Core::Instance ()->GetTorrentInfo (filename);
				if (!info.is_valid ())
					return;
				TrackerURL_->setText (QString::fromStdString (info.trackers ().at (0).url));
				Size_->setText (Util::MakePrettySize (info.total_size ()));

				QString creator = QString::fromUtf8 (info.creator ().c_str ()),
						comment = QString::fromUtf8 (info.comment ().c_str ());

				boost::optional<boost::posix_time::ptime> maybeDate = info.creation_date ();
				QString date;
				if (maybeDate)
					date = QString::fromStdString (boost::posix_time::to_simple_string (*maybeDate.get_ptr ()));

				if (!creator.isEmpty () && !creator.isNull ())
					Creator_->setText (creator);
				else
					Creator_->setText ("<>");
				if (!comment.isEmpty () && !comment.isNull ())
					Comment_->setText (comment);
				else
					Comment_->setText ("<>");
				if (!date.isEmpty () && !date.isNull ())
					Date_->setText (date);
				else
					Date_->setText ("<>");
				FilesModel_->ResetFiles (info.begin_files (), info.end_files ());
				FilesView_->expandAll ();
			}
			
		};
	};
};


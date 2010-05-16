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

#include "importbinary.h"
#include <boost/bind.hpp>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			ImportBinary::ImportBinary (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				on_Browse__released ();
			}
			
			ImportBinary::~ImportBinary ()
			{
			}
			
			QString ImportBinary::GetFilename () const
			{
				return Ui_.File_->text ();
			}
			
			QString ImportBinary::GetTags () const
			{
				return Ui_.AdditionalTags_->text ().trimmed ();
			}
			
			feeds_container_t ImportBinary::GetSelectedFeeds () const
			{
				feeds_container_t result;

				QMap<IDType_t, IDType_t> foreignIDs2Local;
			
				for (int i = 0, end = Ui_.FeedsToImport_->topLevelItemCount ();
						i < end; ++i)
				{
					if (Ui_.FeedsToImport_->topLevelItem (i)->checkState (0) !=
							Qt::Checked)
						continue;
			
					Channel_ptr chan = Channels_ [i];
					if (!foreignIDs2Local.contains (chan->FeedID_))
					{
						Feed_ptr feed (new Feed ());
						feed->LastUpdate_ = QDateTime::currentDateTime ();
						result.push_back (feed);
						foreignIDs2Local [chan->FeedID_] = feed->FeedID_;
					}

					IDType_t our = foreignIDs2Local [chan->FeedID_];
					feeds_container_t::iterator pos =
						std::find_if (result.begin (), result.end (),
								boost::bind (&Feed::FeedID_, _1) == our);
					(*pos)->Channels_.push_back (chan);
				}
			
				return result;
			}
			
			void ImportBinary::on_File__textEdited (const QString& newFilename)
			{
				Reset ();
			
				if (QFile (newFilename).exists ())
					Ui_.ButtonBox_->button (QDialogButtonBox::Open)->
						setEnabled (HandleFile (newFilename));
				else
					Reset ();
			}
			
			void ImportBinary::on_Browse__released ()
			{
				QString startingPath = QFileInfo (Ui_.File_->text ()).path ();
				if (startingPath.isEmpty ())
					startingPath = QDir::homePath ();
			
				QString filename = QFileDialog::getOpenFileName (this,
						tr ("Select binary file"),
						startingPath,
						tr ("Aggregator exchange files (*.lcae);;"
							"All files (*.*)"));
			
				if (filename.isEmpty ())
				{
					QTimer::singleShot (0,
							this,
							SLOT (reject ()));
					return;
				}
			
				Reset ();
			
				Ui_.File_->setText (filename);
			
				Ui_.ButtonBox_->button (QDialogButtonBox::Open)->
					setEnabled (HandleFile (filename));
			}
			
			bool ImportBinary::HandleFile (const QString& filename)
			{
				QFile file (filename);
				if (!file.open (QIODevice::ReadOnly))
				{
					QMessageBox::critical (this,
							tr ("LeechCraft"),
							tr ("Could not open file %1 for reading.")
								.arg (filename));
					return false;
				}
			
				QByteArray buffer = qUncompress (file.readAll ());
				QDataStream stream (&buffer, QIODevice::ReadOnly);
			
				int magic = 0;
				stream >> magic;
				if (magic != static_cast<int> (0xd34df00d))
				{
					QMessageBox::warning (this,
							tr ("LeechCraft"),
							tr ("Selected file %1 is not a valid "
								"LeechCraft::Aggregator exchange file.")
							.arg (filename));
					return false;
				}
			
				int version = 0;
				stream >> version;
			
				if (version != 1)
				{
					QMessageBox::warning (this,
							tr ("LeechCraft"),
							tr ("Selected file %1 is a valid LeechCraft::Aggregator "
								"exchange file, but its version %2 is unknown")
							.arg (filename)
							.arg (version));
				}
			
				QString title, owner, ownerEmail;
				stream >> title >> owner >> ownerEmail;
			
				while (stream.status () == QDataStream::Ok)
				{
					Channel_ptr channel (new Channel (-1, -1));
					stream >> (*channel);
					Channels_.push_back (channel);
			
					QStringList strings (channel->Title_);
					strings << QString::number (channel->Items_.size ());
			
					QTreeWidgetItem *item =
						new QTreeWidgetItem (Ui_.FeedsToImport_, strings);
			
					item->setCheckState (0, Qt::Checked);
				}
			
				return true;
			}
			
			void ImportBinary::Reset ()
			{
				Channels_.clear ();
				Ui_.FeedsToImport_->clear ();
			
				Ui_.ButtonBox_->button (QDialogButtonBox::Open)->setEnabled (false);
			}
		};
	};
};


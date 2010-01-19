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

#include <QFileDialog>
#include <QDir>
#include "firststep.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			FirstStep::FirstStep (QWidget *parent)
			: QWizardPage (parent)
			{
				setupUi (this);
				registerField ("Output", Output_);
				registerField ("AnnounceURL*", AnnounceURL_);
				registerField ("Date", Date_);
				registerField ("Comment", Comment_);
				registerField ("RootPath", RootPath_);
				Date_->setDateTime (QDateTime::currentDateTime ());
				Output_->setText (XmlSettingsManager::Instance ()->
						property ("LastMakeTorrentDirectory").toString ());
				RootPath_->setText (XmlSettingsManager::Instance ()->
						property ("LastAddDirectory").toString ());
				connect (RootPath_,
						SIGNAL (textChanged (const QString&)),
						this,
						SIGNAL (completeChanged ()));
			}

			bool FirstStep::isComplete () const
			{
				QFileInfo info (RootPath_->text ());
				return info.exists () &&
					info.isReadable () &&
					AnnounceURL_->text ().size ();
			}

			QString FirstStep::PrepareDirectory () const
			{
				QString directory = RootPath_->text ();
				if (!QFileInfo (directory).isDir ())
					directory = QFileInfo (directory).absolutePath ();

				if (!QFileInfo (directory).exists ())
					directory = QDir::homePath ();

				if (!directory.endsWith ('/'))
					directory.append ('/');

				return directory;
			}

			void FirstStep::on_BrowseOutput__released ()
			{
				QString last = XmlSettingsManager::Instance ()->
					property ("LastMakeTorrentDirectory").toString ();
				if (!last.endsWith ('/'))
					last += '/';
				if (!QFileInfo (last).exists ())
					last = QDir::homePath ();

				QString directory = QFileDialog::getSaveFileName (this,
						tr ("Select where to save torrent file"),
						last);
				if (directory.isEmpty ())
					return;
			
				Output_->setText (directory);
				XmlSettingsManager::Instance ()->
					setProperty ("LastMakeTorrentDirectory",
							QFileInfo (directory).absolutePath ());
			}
			
			void FirstStep::on_BrowseFile__released ()
			{
				QString path = QFileDialog::getOpenFileName (this,
						tr ("Select torrent contents"),
						PrepareDirectory ());
				if (path.isEmpty ())
					return;
			
				RootPath_->setText (path);
				XmlSettingsManager::Instance ()->
					setProperty ("LastAddDirectory",
							QFileInfo (path).absolutePath ());

				emit completeChanged ();
			}

			void FirstStep::on_BrowseDirectory__released ()
			{
				QString path = QFileDialog::getExistingDirectory (this,
						tr ("Select torrent contents"),
						PrepareDirectory ());
				if (path.isEmpty ())
					return;
			
				RootPath_->setText (path);
				XmlSettingsManager::Instance ()->
					setProperty ("LastAddDirectory",
							path);

				emit completeChanged ();
			}
		};
	};
};


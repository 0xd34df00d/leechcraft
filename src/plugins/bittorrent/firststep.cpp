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
				registerField ("OutputDirectory", OutputDirectory_);
				registerField ("TorrentName*", TorrentName_);
				registerField ("AnnounceURL*", AnnounceURL_);
				registerField ("Date", Date_);
				registerField ("Comment", Comment_);
				registerField ("RootPath", RootPath_);
				Date_->setDateTime (QDateTime::currentDateTime ());
				OutputDirectory_->setText (XmlSettingsManager::Instance ()->
						property ("LastMakeTorrentDirectory").toString ());
				RootPath_->setText (XmlSettingsManager::Instance ()->
						property ("LastAddDirectory").toString ());
			}
			
			void FirstStep::on_BrowseOutput__released ()
			{
				QString directory = QFileDialog::getExistingDirectory (this,
						tr ("Select where to place torrent file"),
						OutputDirectory_->text ());
				if (directory.isEmpty ())
					return;
			
				OutputDirectory_->setText (directory);
				XmlSettingsManager::Instance ()->
					setProperty ("LastMakeTorrentDirectory", directory);
			}
			
			void FirstStep::on_BrowseRoot__released ()
			{
				QString directory = QFileDialog::getExistingDirectory (this,
						tr ("Select root path"),
						RootPath_->text ());
				if (directory.isEmpty ())
					return;
			
				RootPath_->setText (directory);
				XmlSettingsManager::Instance ()->
					setProperty ("LastAddDirectory", directory);
			}
		};
	};
};


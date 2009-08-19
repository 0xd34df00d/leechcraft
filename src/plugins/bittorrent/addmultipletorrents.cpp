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
#include "addmultipletorrents.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			AddMultipleTorrents::AddMultipleTorrents (QWidget *parent)
			: QDialog (parent)
			{
				setupUi (this);
				OpenDirectory_->setText (XmlSettingsManager::Instance ()->
						property ("LastTorrentDirectory").toString ());
				SaveDirectory_->setText (XmlSettingsManager::Instance ()->
						property ("LastSaveDirectory").toString ());
			}
			
			QString AddMultipleTorrents::GetOpenDirectory () const
			{
				return OpenDirectory_->text ();
			}
			
			QString AddMultipleTorrents::GetSaveDirectory () const
			{
				return SaveDirectory_->text ();
			}
			
			Core::AddType AddMultipleTorrents::GetAddType () const
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
			
			LeechCraft::Util::TagsLineEdit* AddMultipleTorrents::GetEdit ()
			{
				return TagsEdit_;
			}
			
			QStringList AddMultipleTorrents::GetTags () const
			{
				return Core::Instance ()->GetProxy ()->GetTagsManager ()->Split (TagsEdit_->text ());
			}
			
			void AddMultipleTorrents::on_BrowseOpen__released ()
			{
				QString dir = QFileDialog::getExistingDirectory (this,
						tr ("Select directory with torrents"),
						OpenDirectory_->text ());
				if (dir.isEmpty ())
					return;
			
				XmlSettingsManager::Instance ()->setProperty ("LastTorrentDirectory", dir);
				OpenDirectory_->setText (dir);
			}
			
			void AddMultipleTorrents::on_BrowseSave__released ()
			{
				QString dir = QFileDialog::getExistingDirectory (this,
						tr ("Select save directory"),
						SaveDirectory_->text ());
				if (dir.isEmpty ())
					return;
			
				XmlSettingsManager::Instance ()->setProperty ("LastSaveDirectory", dir);
				SaveDirectory_->setText (dir);
			}
			
		};
	};
};


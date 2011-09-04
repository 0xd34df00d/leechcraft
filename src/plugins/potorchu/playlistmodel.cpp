/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "playlistmodel.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		PlayListModel::PlayListModel (QObject* parent)
		: QStringListModel (parent)
		{
			const char * const vlc_args[] = {
					"-I", "dummy",
					"--ignore-config",
					"--extraintf=logger",
					"--verbose=2"};
			VLCInstance_ = libvlc_new (sizeof (vlc_args) / sizeof (vlc_args[0]), vlc_args);
		}
		
		PlayListModel::~PlayListModel ()
		{
			libvlc_release (VLCInstance_);
		}
		
		Qt::ItemFlags PlayListModel::flags (const QModelIndex& index) const
		{
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
		}
		
		QVariant PlayListModel::data (const QModelIndex& index, int role) const
		{
			if (role == Qt::DisplayRole)
			{
				libvlc_media_t *t = libvlc_media_new_path (VLCInstance_, data (index, Qt::EditRole).toString ().toAscii ());
				libvlc_media_parse (t);
				QString temp = XmlSettingsManager::Instance ().property ("PlayListTemplate").toString ();
				const QString& artist = QString (libvlc_media_get_meta (t, libvlc_meta_Artist));
				const QString& album = QString (libvlc_media_get_meta (t, libvlc_meta_Album));
				const QString& title = QString (libvlc_media_get_meta (t, libvlc_meta_Title));
				const QString& genre = QString (libvlc_media_get_meta (t, libvlc_meta_Genre));
				const QString& date = QString (libvlc_media_get_meta (t, libvlc_meta_Date));
				temp.replace ("%artist%", artist);
				temp.replace ("%album%", album);
				temp.replace ("%title%", title);
				temp.replace ("%genre%", genre);
				temp.replace ("%date%", date);
				libvlc_media_release (t);

				return temp;
			}
			return QStringListModel::data (index, role);
		}
	}
}


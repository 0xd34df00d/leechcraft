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
		, VLCInstance_ (NULL)
		, ML_ (NULL)
		{
			CurrentIndex_ = -1;
		}
		
		PlayListModel::~PlayListModel ()
		{
		}
		
		bool PlayListModel::SetPlayList (libvlc_media_list_t *ML)
		{
			ML_ = ML;
		}
		
		bool PlayListModel::SetInstance (libvlc_instance_t *VLCInstance)
		{
			VLCInstance_ = VLCInstance;
		}
		
		int PlayListModel::rowCount (const QModelIndex& parent) const
		{
			return libvlc_media_list_count (ML_);
		}
		
		int PlayListModel::CurrentIndex () const
		{
			return CurrentIndex_;
		}
		
		void PlayListModel::SetCurrentIndex (int val)
		{
			CurrentIndex_ = val;
		}
		
		bool PlayListModel::insertRows (int row, const QString& fileName)
		{
			if (!libvlc_media_list_insert_media (ML_,
					libvlc_media_new_path (VLCInstance_, fileName.toAscii ()), row))
			{
				libvlc_media_t *t = libvlc_media_list_item_at_index (ML_, row);
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

				QStringListModel::insertRows (row, 1);
				setData (index (row), temp);
				return true;
			}
			return false;
		}
		
		bool PlayListModel::removeRows (int row)
		{
			if (!libvlc_media_list_remove_index (ML_, row))
			{
				QStringListModel::removeRows (row, 1);
				return true;
			}
			return false;
		}
		
		bool PlayListModel::addItem (const QString& item)
		{
			return insertRows (rowCount (), item);
		}
		
		Qt::ItemFlags PlayListModel::flags (const QModelIndex& index) const
		{
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
		}
	}
}


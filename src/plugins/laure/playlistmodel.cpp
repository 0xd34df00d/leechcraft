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
namespace Laure
{
	PlayListModel::PlayListModel (QObject* parent)
	: QStandardItemModel (parent)
	, VLCInstance_ (NULL)
	, ML_ (NULL)
	{
		CurrentIndex_ = -1;
	}
	
	void PlayListModel::SetPlayList (libvlc_media_list_t *ML)
	{
		ML_ = ML;
	}
	
	void PlayListModel::SetInstance (libvlc_instance_t *VLCInstance)
	{
		VLCInstance_ = VLCInstance;
	}
	
	int PlayListModel::CurrentIndex () const
	{
		return CurrentIndex_;
	}
	
	void PlayListModel::SetCurrentIndex (int val)
	{
		//TODO: a temporary solution. fix it later
		if (val == rowCount ())
			CurrentIndex_ = 0;
		else if (val == -1)
			CurrentIndex_ = rowCount () - 1;
		else
			CurrentIndex_ = val;
	}
	
	libvlc_media_t* PlayListModel::CurrentMedia ()
	{
		return libvlc_media_list_item_at_index (ML_, CurrentIndex_);
	}
	
	void PlayListModel::appendRow (QStandardItem *item)
	{
		libvlc_media_t *t = libvlc_media_new_path (VLCInstance_, item->text ().toAscii ());
		if (!libvlc_media_list_add_media (ML_, t))
		{
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
			
			QStandardItemModel::appendRow (new QStandardItem (temp));
		}
	}
	
	bool PlayListModel::removeRows (int row)
	{
		if (!libvlc_media_list_remove_index (ML_, row))
			return QStandardItemModel::removeRows (row, 1);
		
		return false;
	}
	
	Qt::ItemFlags PlayListModel::flags (const QModelIndex& index) const
	{
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
	}
}
}


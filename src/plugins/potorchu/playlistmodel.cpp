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
		PlayListModel::PlayListModel (libvlc_instance_t *VLCInstance, QObject* parent)
		: QAbstractItemModel (parent)
		, VLCInstance_ (VLCInstance)
		{
			ML_ = libvlc_media_list_new (VLCInstance_);
		}
		
		PlayListModel::~PlayListModel ()
		{
			libvlc_media_list_release (ML_);
		}
		
		libvlc_media_list_t *PlayListModel::GetPlayList ()
		{
			return ML_;
		}
		
		bool PlayListModel::setData (const QModelIndex& index, const QVariant& value, int role)
		{
			return true;
		}
		
		int PlayListModel::rowCount (const QModelIndex& parent) const
		{
			qDebug () << Q_FUNC_INFO <<  libvlc_media_list_count (ML_);
			return libvlc_media_list_count (ML_);
		}
		
		bool PlayListModel::insertRows (int row, int count, const QString& fileName)
		{
			return !libvlc_media_list_insert_media (ML_, libvlc_media_new_path (VLCInstance_, fileName.toAscii ()), row);
		}
		
		bool PlayListModel::removeRows (int row, int count, const QModelIndex& parent)
		{
			return !libvlc_media_list_remove_index (ML_, row);
		}
		
		void PlayListModel::addItem (const QString& item)
		{
			libvlc_media_list_add_media (ML_, libvlc_media_new_path (VLCInstance_, item.toAscii ()));
		}
		
		Qt::ItemFlags PlayListModel::flags (const QModelIndex& index) const
		{
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
		}
		
		QVariant PlayListModel::data (const QModelIndex& index, int role) const
		{
			if (role == Qt::DisplayRole)
			{
				qDebug () << Q_FUNC_INFO;
				libvlc_media_t *t = libvlc_media_list_item_at_index (ML_, index.row ());
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

				return temp;
			}
			return QVariant ("Blah");
		}
		
		QModelIndex PlayListModel::index (int row, int column, const QModelIndex& parent) const
		{
			return createIndex (row, column);
		}
		
		QModelIndex PlayListModel::parent (const QModelIndex& index) const
		{
			return QModelIndex ();
		}
		
		int PlayListModel::columnCount (const QModelIndex& parent) const
		{
			return 1;
		}
	}
}


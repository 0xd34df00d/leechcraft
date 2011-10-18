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

#ifndef PLUGINS_LAURE_PLAYLISTMODEL_H
#define PLUGINS_LAURE_PLAYLISTMODEL_H
#include <QStandardItemModel>
#include <vlc/vlc.h>

namespace LeechCraft
{
namespace Laure
{
	class PlayListModel : public QStandardItemModel
	{
		Q_OBJECT
		
		libvlc_instance_t *VLCInstance_;
		libvlc_media_list_t *ML_;
		int CurrentIndex_;
	public:
		PlayListModel (QObject *);
		
		void SetPlayList (libvlc_media_list_t*);
		void SetInstance (libvlc_instance_t*);
		
		int CurrentIndex () const;
		libvlc_media_t* CurrentMedia ();
		void SetCurrentIndex (int);
		
		bool insertRows (int row, const QString& fileName);
		bool removeRows (int);
		Qt::ItemFlags flags (const QModelIndex&) const;
		
		void appendRow (QStandardItem*);
	};
}
}

#endif // PLUGINS_LAURE_PLAYLISTMODEL_H

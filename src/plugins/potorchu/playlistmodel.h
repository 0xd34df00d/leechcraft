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

#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QStringListModel>
#include <QStringList>
#include <vlc/vlc.h>
#include <QDebug>

namespace LeechCraft
{
	namespace Potorchu
	{
		class PlayListModel : public QAbstractItemModel
		{
			Q_OBJECT
			libvlc_instance_t *VLCInstance_;
			libvlc_media_list_t *ML_;
		public:
			PlayListModel (libvlc_instance_t *VLCInstance, QObject *parent);
			virtual ~PlayListModel ();
			libvlc_media_list_t *GetPlayList ();
			int rowCount (const QModelIndex& parent = QModelIndex ()) const;
			bool insertRows (int row, int count, const QString& fileName);
			bool removeRows (int row, int count, const QModelIndex& parent = QModelIndex ());
			bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
			Qt::ItemFlags flags (const QModelIndex& index) const;
			QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
			
			QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex ()) const;
			QModelIndex parent (const QModelIndex& index) const;
			int columnCount (const QModelIndex& parent = QModelIndex ()) const;
		public slots:
			void addItem (const QString& item);
		};
	}
}

#endif // PLAYLISTMODEL_H

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

#include "playlistview.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		PlayListView::PlayListView (QWidget *parent)
		: QListView (parent)
		, PlayListModel_ (NULL)
		{
			connect (this,
					SIGNAL (doubleClicked (QModelIndex)),
					this,
					SLOT (handleDoubleClicked (QModelIndex)));
		}
		
		void PlayListView::Init (libvlc_instance_t *VLCInstance)
		{
			PlayListModel_ = new PlayListModel (VLCInstance, this);
			setModel (PlayListModel_);
			setModelColumn (0);
		}
		
		libvlc_media_list_t *PlayListView::GetMediaList ()
		{
			return PlayListModel_->GetPlayList ();
		}

		void PlayListView::addItem (const QString& item)
		{
			PlayListModel_->addItem (item);
		}
		
		void PlayListView::handleDoubleClicked (const QModelIndex& index)
		{
			emit playItem (index.row ());
		}
		
		void PlayListView::removeSelectedRows ()
		{
			const QModelIndexList& indexList = selectedIndexes ();
			Q_FOREACH (const QModelIndex& index, indexList)
				PlayListModel_->removeRow (index.row ());
		}
	}
}
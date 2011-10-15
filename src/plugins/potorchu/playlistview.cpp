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
		{
			PlayListModel_ = new PlayListModel (this);
			setModel (PlayListModel_);
			setModelColumn (0);
			connect (this,
					SIGNAL (doubleClicked (QModelIndex)),
					this,
					SLOT (handleDoubleClicked (QModelIndex)));
		}
		
		bool PlayListView::SetPlayList (libvlc_media_list_t *ML)
		{
			return PlayListModel_->SetPlayList (ML);
		}
		
		int PlayListView::RowCount () const
		{
			return PlayListModel_->rowCount ();
		}
		
		bool PlayListView::SetInstance (libvlc_instance_t *VLCInstance)
		{
			return PlayListModel_->SetInstance (VLCInstance);
		}
		
		int PlayListView::CurrentIndex () const
		{
			return PlayListModel_->CurrentIndex ();
		}
		
		void PlayListView::SetCurrentIndex (int val)
		{
			PlayListModel_->SetCurrentIndex (val);
			setCurrentIndex (PlayListModel_->index (val));
			//emit playItem (CurrentIndex ());
			//BUG: Don't uncomment it
			//if (val >= 0 && val < RowCount ())
			//	emit playItem (val);
		}

		void PlayListView::addItem (const QString& item)
		{
			if (PlayListModel_->addItem(item) && PlayListModel_->rowCount () == 1)
			{
				SetCurrentIndex(0);
				emit playItem (0);
			}
		}
		
		libvlc_media_t *PlayListView::CurrentMedia ()
		{
			return PlayListModel_->CurrentMedia ();
		}
		
		void PlayListView::handleDoubleClicked (const QModelIndex& index)
		{
			emit playItem (index.row ());
		}
		
		void PlayListView::removeSelectedRows ()
		{
			const QModelIndexList& indexList = selectedIndexes ();
			Q_FOREACH (const QModelIndex& index, indexList)
				PlayListModel_->removeRows (index.row ());
		}
		
		void PlayListView::keyPressEvent (QKeyEvent *event)
		{
			const QModelIndex& curIndex = currentIndex ();
			switch (event->key ())
			{
			case Qt::Key_Delete:
				removeSelectedRows ();
				break;
			case Qt::Key_Return:
				emit playItem (curIndex.row ());
				break;
			case Qt::Key_Up:
				setCurrentIndex (model ()->index (curIndex.row () - 1, curIndex.column ()));
				break;
			case Qt::Key_Down:
				setCurrentIndex (model ()->index (curIndex.row () + 1, curIndex.column ()));
				break;
			}
		}
		
		void PlayListView::moveSelect (int x, int y)
		{
			if ( x < 0 || y < 1 || x >= model ()->rowCount ()
					|| y >= model ()->columnCount ())
				return;
			setCurrentIndex (model ()->index (x, y));
		}
	}
}
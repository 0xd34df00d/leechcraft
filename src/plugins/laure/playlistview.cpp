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

#include <QKeyEvent>

namespace LeechCraft
{
namespace Laure
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
	
	void PlayListView::SetPlayList (libvlc_media_list_t *ML)
	{
		PlayListModel_->SetPlayList (ML);
	}
	
	int PlayListView::RowCount () const
	{
		return PlayListModel_->rowCount ();
	}
	
	void PlayListView::SetInstance (libvlc_instance_t *VLCInstance)
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
		setCurrentIndex (PlayListModel_->index (val, 0	));
	}

	void PlayListView::AddItem (const QString& item)
	{
		PlayListModel_->appendRow (new QStandardItem (item));
		if (PlayListModel_->rowCount () == 1)
		{
			SetCurrentIndex (0);
			emit itemPlayed (0);
		}
	}
	
	libvlc_media_t* PlayListView::CurrentMedia ()
	{
		return PlayListModel_->CurrentMedia ();
	}
	
	void PlayListView::handleDoubleClicked (const QModelIndex& index)
	{
		emit itemPlayed (index.row ());
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
			emit itemPlayed (curIndex.row ());
			break;
		case Qt::Key_Up:
			setCurrentIndex (model ()->index (curIndex.row () - 1,
							curIndex.column ()));
			break;
		case Qt::Key_Down:
			setCurrentIndex (model ()->index (curIndex.row () + 1,
							curIndex.column ()));
			break;
		}
	}
	
	void PlayListView::MoveSelect (int x, int y)
	{
		if ( x < 0 || y < 1 || x >= model ()->rowCount ()
				|| y >= model ()->columnCount ())
			return;
		setCurrentIndex (model ()->index (x, y));
	}
}
}	
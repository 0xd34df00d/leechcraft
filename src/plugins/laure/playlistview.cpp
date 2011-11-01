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
#include <QHeaderView>
#include <QDebug>
#include "xmlsettingsmanager.h"
#include "nowplayingdelegate.h"

namespace LeechCraft
{
namespace Laure
{
	PlayListView::PlayListView (QWidget *parent)
	: QTreeView (parent)
	, PlayListModel_ (new PlayListModel (this))
	, CurrentItem_ (-1)
	{
		setModel (PlayListModel_);
		setSelectionMode (ContiguousSelection);
		setAlternatingRowColors (true);
		hideColumn (1);
		setItemDelegate (new NowPlayingDelegate (this));
		header ()->hide ();
		connect (this,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handleDoubleClicked (QModelIndex)));
	}
	
	QVariant PlayListView::Data (int row, int column)
	{
		return PlayListModel_->data (PlayListModel_->index (row, column));
	}
	
	int PlayListView::RowCount () const
	{
		return PlayListModel_->rowCount ();
	}
	
	void PlayListView::selectRow (int val)
	{
		setCurrentIndex (model ()->index (val, 0));
	}

	void PlayListView::AddItem (const MediaMeta& item, const QString& fileName)
	{
		QString format = XmlSettingsManager::Instance ()
				.property ("PlaylistFormat").toString ();
		format.replace ("%artist%", item.Artist_);
		format.replace ("%album%", item.Album_);
		format.replace ("%title%", item.Title_);
		format.replace ("%genre%", item.Genre_);
		format.replace ("%date%", item.Date_);
		
		QList<QStandardItem*> list;
		list << new QStandardItem (format);
		list << new QStandardItem (fileName);
		PlayListModel_->appendRow (list);
	}
	
	void PlayListView::Play (int row)
	{
		QStandardItem *it = PlayListModel_->item (CurrentItem_);
		if (it)
			it->setData (false, PlayListModel::IsPlayingRole);
		it = PlayListModel_->item (row);
		it->setData (true, PlayListModel::IsPlayingRole);
		CurrentItem_ = row;
	}

	void PlayListView::handleDoubleClicked (const QModelIndex& index)
	{
		emit playItem (index.row ());
	}
	
	void PlayListView::removeSelectedRows ()
	{
		const QModelIndexList& indexList = selectedIndexes ();
		const int c = indexList.count ();
		if (!c)
			return;
		
		const int first = indexList.first ().row ();
		PlayListModel_->removeRows (first, indexList.count ());
		for (int i = c - 1; i > -1; --i)
			emit itemRemoved (first);
	
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
			setCurrentIndex (model ()->index (curIndex.row () - 1, 0));
			break;
		case Qt::Key_Down:
			setCurrentIndex (model ()->index (curIndex.row () + 1, 0));
			break;
		}
	}
}
}	
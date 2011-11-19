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
#include <QStandardItemModel>
#include <QMenu>
#include "nowplayingdelegate.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Laure
{
	const int PlayListColumnCount = 6;
	
	PlayListView::PlayListView (QStandardItemModel *model, QWidget *parent)
	: QTreeView (parent)
	, PlayListModel_ (model)
	, CurrentItem_ (-1)
	{
		setModel (PlayListModel_);
		
		PlayListModel_->setColumnCount (PlayListColumnCount);
		
		setEditTriggers (SelectedClicked);
		setSelectionMode (ContiguousSelection);
		setAlternatingRowColors (true);
		hideColumn (0);

		handleHideHeaders ();
		
		QList<QByteArray> propNames;
		
		for (int i = 0; i < PlayListModel_->columnCount (); ++i)
			propNames.push_back ("Header" + QString::number (i).toAscii ());
		
		XmlSettingsManager::Instance ().RegisterObject (propNames, this,
				"handleHideHeaders");

		setItemDelegate (new NowPlayingDelegate (this));
		setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
		
		header ()->setResizeMode (QHeaderView::ResizeToContents);
		
		connect (this,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handleDoubleClicked (QModelIndex)));
	}
	
	void PlayListView::handleHideHeaders ()
	{
		for (int i = 1; i < PlayListModel_->columnCount (); ++i)
		{
			const QString& itemName = "Header" + QString::number (i);
			setColumnHidden (i, !XmlSettingsManager::Instance ()
					.property (itemName.toAscii ()).toBool ());
		}
	}
	
	void PlayListView::selectRow (int row)
	{
		setCurrentIndex (model ()->index (row, 0));
	}

	void PlayListView::AddItem (const MediaMeta& item, const QString& fileName)
	{
		QList<QStandardItem*> list;
		list << new QStandardItem (fileName);
		list << new QStandardItem (item.Artist_);
		list << new QStandardItem (item.Title_);
		list << new QStandardItem (item.Album_);
		list << new QStandardItem (item.Genre_);
		list << new QStandardItem (item.Date_);
		
		Q_FOREACH (QStandardItem *itemList, list)
			itemList->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled
					| Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
			
		PlayListModel_->appendRow (list);
	}
	
	void PlayListView::MarkPlayingItem (int row)
	{
		auto it = PlayListModel_->item (CurrentItem_);
		if (it)
			it->setData (false, Roles::IsPlayingRole);
		it = PlayListModel_->item (row);
		it->setData (true, Roles::IsPlayingRole);
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
		PlayListModel_->removeRows (first, indexList.count ()
				/ (PlayListModel_->columnCount () - 1));
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
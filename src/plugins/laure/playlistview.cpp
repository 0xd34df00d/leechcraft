/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QTime>
#include "nowplayingdelegate.h"
#include "xmlsettingsmanager.h"
#include "playliststatusdelegate.h"

namespace LeechCraft
{
namespace Laure
{	
	PlayListView::PlayListView (QStandardItemModel *model, QWidget *parent)
	: QTreeView (parent)
	, PlayListModel_ (model)
	, CurrentItem_ (-1)
	{
		setModel (PlayListModel_);
		
		PlayListModel_->setColumnCount (PlayListColumns::MAX);
		
		setEditTriggers (SelectedClicked);
		setSelectionMode (ContiguousSelection);
		setAlternatingRowColors (true);
		hideColumn (URLColumn);
		
		HeaderProperties_ [ArtistColumn] = "ArtistHeader";
		HeaderProperties_ [TitleColumn] = "TitleHeader";
		HeaderProperties_ [AlbumColumn] = "AlbumHeader";
		HeaderProperties_ [GenreColumn] = "GenreHeader";
		HeaderProperties_ [DateColumn] = "DateHeader";
		
		handleHideHeaders ();
		
		QList<QByteArray> propNames;
		Q_FOREACH (const QByteArray& el, HeaderProperties_)
			propNames << el;
		
		XmlSettingsManager::Instance ().RegisterObject (propNames, this,
				"handleHideHeaders");

		setItemDelegate (new NowPlayingDelegate (this));
		
		header ()->setResizeMode (QHeaderView::Interactive);
		header ()->setContextMenuPolicy (Qt::CustomContextMenu);
		connect (header (),
				SIGNAL (sectionResized (int, int, int)),
				this,
				SLOT (handleSectionResized (int, int, int)));
		
		setColumnWidth (StatusColumn, 40);
		header ()->setResizeMode (StatusColumn, QHeaderView::Fixed);
		header ()->setResizeMode (QueueColumn, QHeaderView::ResizeToContents);
		header ()->setResizeMode (LengthColumn, QHeaderView::ResizeToContents);

		setItemDelegateForColumn (PlayListColumns::StatusColumn, new PlayListStatusDelegate (this));
		setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
		
		connect (header (),
				SIGNAL (customContextMenuRequested (QPoint)),
				this,
				SLOT (handleHeaderMenu (QPoint)));
		
		setContextMenuPolicy (Qt::CustomContextMenu);
		connect (this,
				SIGNAL (customContextMenuRequested (QPoint)),
				this,
				SLOT (handleMenu (QPoint)));
		
		connect (this,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handleDoubleClicked (QModelIndex)));
		
		QStringList headers;
		
		headers << QString ()
				<< QString ()
				<< tr ("Artist")
				<< tr ("Title")
				<< tr ("Album")
				<< tr ("Genre")
				<< tr ("Date")
				<< QString ()
				<< tr ("Length");
				
		PlayListModel_->setHorizontalHeaderLabels (headers);
		
		for (int i = ArtistColumn; i < QueueColumn; ++i)
		{
			const int prop = XmlSettingsManager::Instance ()
					.property ("PlayListHeader" + QString::number (i).toAscii ()).toInt ();
			if (!prop)
				continue;
			setColumnWidth (i, prop);
		}
	}
	
	void PlayListView::Init (std::shared_ptr<VLCWrapper> wrapper)
	{
		VLCWrapper_ = wrapper;
		VLCWrapper *w = wrapper.get ();
		connect (this,
				SIGNAL (itemRemoved (int)),
				w,
				SLOT (removeRow (int)));
		connect (this,
				SIGNAL (playItem (int)),
				w,
				SLOT (playItem (int)));
	}
	
	void PlayListView::handleSectionResized (int logicalIndex,
			int oldSize, int newSize)
	{
		XmlSettingsManager::Instance ().setProperty ("PlayListHeader" +
				QString::number (logicalIndex).toAscii (), newSize);
	}
	
	void PlayListView::handleHeaderMenu (const QPoint& point)
	{
		QMenu menu;
		for (int i = ArtistColumn; i < QueueColumn; ++i)
		{
			QAction *menuAction = new QAction (header ()->model ()->
					headerData (i, Qt::Horizontal).toString (), &menu);
			menuAction->setCheckable (true);
			menuAction->setData (i);
			
			if (!isColumnHidden (i))
				menuAction->setChecked (true);
			menu.addAction (menuAction);
		}
		
		QAction *selectedItem = menu.exec (mapToGlobal (point));
		if (selectedItem)
		{
			PlayListColumns columnIndex = static_cast<PlayListColumns> (selectedItem->data ().toInt ());
			XmlSettingsManager::Instance ().setProperty (HeaderProperties_ [columnIndex],
						selectedItem->isChecked ());
		}
	}
	
	void PlayListView::handleMenu (const QPoint& point)
	{
		if (selectedIndexes ().empty ())
			return;
		
		const int row = selectedIndexes ().first ().row ();
		
		const bool found = VLCWrapper_->GetQueueListIndexes ().contains (row);
		
		QMenu menu;
		QAction *menuAction = new QAction (found ? tr ("Unqueue") : tr ("Queue"), &menu);
		menuAction->setData (found);
		
		menu.addAction (menuAction);
		QAction *action = menu.exec (mapToGlobal (point));
		if (!action)
			return;
		
		if (action->data ().toBool ())
			VLCWrapper_->removeFromQueue (row);
		else
			VLCWrapper_->addToQueue (row);
		
		UpdateQueueIndexes ();
	}
	
	void PlayListView::UpdateQueueIndexes ()
	{
		const QList<int>& queueIndexes = VLCWrapper_->GetQueueListIndexes ();
		
		for (int i = 0; i < PlayListModel_->columnCount (); ++i)
			PlayListModel_->setData (PlayListModel_->
					index (i, PlayListColumns::QueueColumn), QString ());
		
		int i = 0;
		Q_FOREACH (const int index, queueIndexes)
			PlayListModel_->setData (PlayListModel_->
					index (index, PlayListColumns::QueueColumn), "#" + QString::number (i++));
	}
	
	void PlayListView::handleHideHeaders ()
	{
		NotHiddenColumnCount_ = 0;
		for (int i = ArtistColumn; i < QueueColumn; ++i)
		{
			const bool checked = XmlSettingsManager::Instance ()
					.property (HeaderProperties_ [static_cast<PlayListColumns> (i)]).toBool ();
			if (checked)
				++NotHiddenColumnCount_;
			
			setColumnHidden (i, !checked);
		}
	}
	
	void PlayListView::selectRow (int row)
	{
		setCurrentIndex (model ()->index (row, 0));
	}

	void PlayListView::AddItem (const MediaMeta& item, const QString& fileName)
	{
		QList<QStandardItem*> list;
		list << new QStandardItem (fileName)
				<< new QStandardItem (item.Artist_)
				<< new QStandardItem (item.Title_)
				<< new QStandardItem (item.Album_)
				<< new QStandardItem (item.Genre_)
				<< new QStandardItem (item.Date_);
		
		Q_FOREACH (QStandardItem *itemList, list)
			itemList->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled
					| Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
		
		QStandardItem *queueItem = new QStandardItem ();
		queueItem->setFlags (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		
		QStandardItem *timeItem = new QStandardItem (QTime ()
				.addSecs (item.Length_).toString ());
		timeItem->setFlags (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		
		list << queueItem << timeItem;
		
		QStandardItem *statusItem = new QStandardItem ();
		statusItem->setFlags (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		list.push_front (statusItem);
		
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
		if (!c || !NotHiddenColumnCount_)
			return;
		
		const int first = indexList.first ().row ();
		const int rows = indexList.count () / NotHiddenColumnCount_;
		PlayListModel_->removeRows (first, rows);
		for (int i = 0; i < rows; ++i)
			emit itemRemoved (first);
		
		UpdateQueueIndexes ();
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
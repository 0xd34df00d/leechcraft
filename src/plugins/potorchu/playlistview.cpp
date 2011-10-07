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
		, PlayListModel_ (new PlayListModel (this))
		{
			setModel (PlayListModel_);
			setModelColumn (0);
			connect (this,
					SIGNAL (doubleClicked (QModelIndex)),
					this,
					SLOT (handleDoubleClicked (QModelIndex)));
		}
		
		void PlayListView::nextFile ()
		{
			const QModelIndex& index = currentIndex ();
			emit play (PlayListModel_->index (index.row () + 1).data (Qt::EditRole).toString ());
			setCurrentIndex (PlayListModel_->index (index.row () + 1));
		}

		void PlayListView::addItem (const QString& item)
		{
			const int rowc = PlayListModel_->rowCount ();
			PlayListModel_->insertRows (rowc, 1);
			PlayListModel_->setData (PlayListModel_->index (rowc, 0), item);
		}
		
		void PlayListView::handleDoubleClicked (const QModelIndex& index)
		{
			emit play (index.data (Qt::EditRole).toString ());
		}
		
		void PlayListView::removeSelectedRows ()
		{
			const QModelIndexList& indexList = selectedIndexes ();
			Q_FOREACH (const QModelIndex& index, indexList)
				PlayListModel_->removeRow (index.row ());
		}
	}
}
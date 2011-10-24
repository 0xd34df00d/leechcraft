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

#include "playlistwidget.h"
#include <QToolBar>
#include <vlc/vlc.h>
#include "chooseurldialog.h"
#include "playlistaddmenu.h"

namespace LeechCraft
{
namespace Laure
{
	PlayListWidget::PlayListWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		setVisible (false);
		
		QAction *actionAdd = new QAction (tr ("Add"), this);
		QAction *actionRemove = new QAction (tr ("Remove"), this);
		PlayListAddMenu *menu = new PlayListAddMenu (this);
		
		actionAdd->setProperty ("ActionIcon", "add");
		actionRemove->setProperty ("ActionIcon", "remove");
		
		actionAdd->setMenu (menu);
		actionAdd->setMenuRole (QAction::ApplicationSpecificRole);
		
		
		QToolBar *actionBar = new QToolBar (Ui_.ActionFrame_);
		actionBar->setToolButtonStyle (Qt::ToolButtonIconOnly);
		actionBar->setIconSize (QSize (16, 16));
		actionBar->addAction (actionAdd);
		actionBar->addAction (actionRemove);
		
		connect (actionRemove,
				SIGNAL (triggered (bool)),
				Ui_.PlayListView_,
				SLOT (removeSelectedRows ()));
		connect (menu,
				SIGNAL (addItem (QString)),
				this,
				SIGNAL (itemAddedRequest (QString)));
		connect (Ui_.PlayListView_,
				SIGNAL (itemRemoved (int)),
				this,
				SIGNAL (itemRemoved (int)));
		connect (Ui_.PlayListView_,
				SIGNAL (playItem (int)),
				this,
				SIGNAL (playItem (int)));
	}
	
	void PlayListWidget::handleItemPlayed (int row)
	{
		Ui_.PlayListView_->selectRow (row);
		Ui_.PlayListView_->Play (row);
	}
	
	void PlayListWidget::handleItemAdded (const MediaMeta& meta)
	{
		Ui_.PlayListView_->AddItem (meta);
	}
}
}

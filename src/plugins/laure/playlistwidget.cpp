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
#include <QFileDialog>
#include <QDockWidget>
#include <QTextStream>
#include <vlc/vlc.h>
#include <util/util.h>
#include "chooseurldialog.h"
#include "playlistaddmenu.h"
#include "playlistmodel.h"

namespace LeechCraft
{
namespace Laure
{
	PlayListWidget::PlayListWidget (QWidget *parent)
	: QWidget (parent)
	, GridLayout_ (new QGridLayout (this))
	, PlayListModel_ (new PlayListModel (this))
	, PlayListView_ (new PlayListView (PlayListModel_, this))
	, ActionBar_ (new QToolBar (this))
	{
		setLayout (GridLayout_);
		setVisible (false);
		
		GridLayout_->addWidget (PlayListView_, 0, 0);
		
		GridLayout_->addWidget (ActionBar_, 1, 0);
		GridLayout_->setSpacing (0);
		GridLayout_->setMargin (0);
		
		QAction *actionAdd = new QAction (tr ("Add"), this);
		QAction *actionRemove = new QAction (tr ("Remove"), this);
		QAction *exportAction = new QAction (tr ("Export to m3u"), this);
		
		PlayListAddMenu *menu = new PlayListAddMenu (this);
		
		actionAdd->setProperty ("ActionIcon", "add");
		actionRemove->setProperty ("ActionIcon", "remove");
		exportAction->setProperty ("ActionIcon", "documentsaveas");
		
		actionAdd->setMenu (menu);
		actionAdd->setMenuRole (QAction::ApplicationSpecificRole);
		
		ActionBar_->setToolButtonStyle (Qt::ToolButtonIconOnly);
		ActionBar_->setIconSize (QSize (16, 16));
		ActionBar_->addAction (actionAdd);
		ActionBar_->addAction (actionRemove);
		ActionBar_->addAction (exportAction);

		connect (actionRemove,
				SIGNAL (triggered (bool)),
				PlayListView_,
				SLOT (removeSelectedRows ()));
		connect (exportAction,
				SIGNAL (triggered (bool)),
				this,
				SLOT (handleExportPlayList ()));
		connect (menu,
				SIGNAL (addItem (QString)),
				this,
				SIGNAL (itemAddedRequest (QString)));
		connect (PlayListView_,
				SIGNAL (itemRemoved (int)),
				this,
				SIGNAL (itemRemoved (int)));
		connect (PlayListView_,
				SIGNAL (playItem (int)),
				this,
				SIGNAL (playItem (int)));
	}
	
	void PlayListWidget::handleItemPlayed (int row)
	{
		PlayListView_->selectRow (row);
		PlayListView_->Play (row);
	}
	
	void PlayListWidget::handleItemAdded (const MediaMeta& meta,
			const QString& fileName)
	{
		PlayListView_->AddItem (meta, fileName);
	}
	
	void PlayListWidget::handleExportPlayList ()
	{
		const QString& fileName = QFileDialog::getSaveFileName (this,
				tr ("Save to playlist"), QDir::homePath (),
				"*.m3u");
		if (fileName.isEmpty ())
			return;
		
		QFile file (fileName);
		if (!file.open (QIODevice::WriteOnly | QIODevice::Text))
		{
			emit gotEntity (Util::MakeNotification ("Laure",
					tr ("Can't export %1")
							.arg (fileName),
					PInfo_));
			return;
		}
		QTextStream out (&file);
		out << "#EXTM3U\n";
		
		for (int i = 0, c = PlayListModel_->rowCount (); i < c; ++i)
			out << PlayListModel_->data (PlayListModel_->index (i, 0))
					.toString () << '\n';
	}
}
}

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
#include <QToolButton>
#include <QMenu>
#include <QFileDialog>

#include "chooseurldialog.h"

namespace LeechCraft
{
	namespace Potorchu
	{
		PlayListWidget::PlayListWidget (QWidget *parent)
		: QWidget (parent)
		, Ui_ (new Ui::PlayListWidget) 
		{
			Ui_->setupUi (this);
			ActionBar_ = new QToolBar (Ui_->ActionFrame_);
			Ui_->ActionFrame_->setFrameStyle (QFrame::NoFrame);
			ActionBar_->setToolButtonStyle (Qt::ToolButtonIconOnly);
			ActionBar_->setIconSize (QSize (16, 16));
		}
		
		PlayListWidget::~PlayListWidget ()
		{
			delete Ui_;
		}
		
		PlayListView* PlayListWidget::GetPlayListView ()
		{
			return Ui_->PlayListView_;
		}
		
		void PlayListWidget::Init (ICoreProxy_ptr proxy)
		{
			QToolButton *actionAdd = new QToolButton (this);
			actionAdd->setIcon (proxy->GetIcon ("add"));
			actionAdd->setPopupMode (QToolButton::InstantPopup);
			ActionBar_->addWidget (actionAdd);
			QMenu *addMenu = new QMenu (this);
			
			QAction *addFiles = new QAction (tr ("Add files"), this);
			QAction *addFolder = new QAction (tr ("Add folder"), this);
			QAction *addURL = new QAction (tr ("Add URL"), this);
			
			addMenu->addAction (addFiles);
			addMenu->addAction (addFolder);
			addMenu->addAction (addURL);
			
			actionAdd->setMenu (addMenu);
			QAction *actionRemove = new QAction (proxy->GetIcon ("remove"),
					tr ("Remove"), this);
			ActionBar_->addAction (actionRemove);
			
			connect (addURL,
					SIGNAL (triggered (bool)),
					this,
					SLOT (handleAddUrl ()));
			connect (addFiles,
					SIGNAL (triggered (bool)),
					this,
					SLOT (handleAddFiles ()));
			connect (addFolder,
					SIGNAL (triggered (bool)),
					this,
					SLOT (handleAddFolder ()));
		}
		
		void PlayListWidget::handleAddFiles ()
		{
			const QString& fileName = QFileDialog::getOpenFileNames (this,
					tr ("Choose file"), QDir::homePath ());
			if (!fileName.isEmpty ())
				Ui_->PlayListView_->addItem (fileName);
		}

		void PlayListWidget::handleAddFolder ()
		{
				//TODO Recursive directory parsing
		}
		
		void PlayListWidget::handleAddUrl ()
		{
			ChooseURLDialog *d = new ChooseURLDialog (this);
			if (d->exec () == QDialog::Accepted)
			{
				const QString& url = d->GetUrl ();
				Ui_->PlayListView_->addItem (url);
			}
		}


	}
}
